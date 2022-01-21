function setupEnergySamplerTab()
{
    var confpane = getYuboxPane('espnow');

    // https://getbootstrap.com/docs/4.4/components/navs/#events
    getYuboxNavTab('espnow').on('shown.bs.tab', function (e) {
        yuboxLoadLectorConfig();
    });

    confpane.find('button[name=apply]').click(function () {
        var ok = true;
        var msg = '';

        var mac = confpane.find('input#medidor_id').val().trim();
        const myArray = mac.split(":");
        console.log(myArray);

        if (mac == '') {
            ok = false;
            msg = 'Se requiere MAC del servidor';
        }
        else if (myArray.length!=6){
            ok = false;
            msg = 'Formato incorrecto para MAC del servidor';
        }
        else {
            var postData = {
                impl:           confpane.find('input[type=radio][name=impl]:checked').val(),
                interval:       confpane.find('input#interval').val().trim(),
                MAC_1:           parseInt(myArray[0],16),
                MAC_2:           parseInt(myArray[1],16),
                MAC_3:           parseInt(myArray[2],16),
                MAC_4:           parseInt(myArray[3],16),
                MAC_5:           parseInt(myArray[4],16),
                MAC_6:           parseInt(myArray[5],16),                             
            };
            console.log(postData);
        }

        if (!ok) {
            yuboxMostrarAlertText('danger', msg, 2000);
        } else {
            console.log(postData);
            $.post(yuboxAPI('espnow')+'/config.json', postData)
            .done(function (data) {
                if (data.success) {
                    yuboxMostrarAlertText('success', data.msg, 2000);

                    // Recargar luego de 5 segundos para verificar que el muestreador
                    // realmente está corriendo.
                    setTimeout(yuboxLoadLectorConfig, 1 * 1000);
                } else {
                    yuboxMostrarAlertText('danger', data.msg, 2000);
                }
            })
            .fail(function (e) { yuboxStdAjaxFailHandler(e, 2000); });
        }
    });

}

function yuboxLoadLectorConfig()
{
    var confpane = getYuboxPane('espnow');

    confpane.find('span#running')
        .removeClass('badge-success badge-danger')
        .addClass('badge-secondary')
        .text('(consultando)');

    $.getJSON(yuboxAPI('espnow')+'/config.json')
    .done(function (data) {

        if (data.update == ''){
            confpane.find('span#actualizacion')
            .removeClass('badge-success badge-danger badge-secondary')
            .addClass(false ? 'badge-success' : 'badge-danger')
            .text("NUNCA");
        }
        else{
            confpane.find('span#actualizacion')
            .removeClass('badge-success badge-danger badge-secondary')
            .addClass(true ? 'badge-success' : 'badge-danger')
            .text(data.update);
        }

        confpane.find('span#running')
        .removeClass('badge-success badge-danger badge-secondary')
        .addClass(data.running ? 'badge-success' : 'badge-danger')
        .text(data.running ? 'ACTIVO' : 'INACTIVO');


        confpane.find('input#medidor_id').val((data.medidor_id != null) ? data.medidor_id : '');
        confpane.find('input#interval').val(data.interval);
        confpane.find('input[type=radio][name=impl]#impl-'+data.impl).click();

    })
    .fail(function (e) { yuboxStdAjaxFailHandler(e, 2000); });
}
