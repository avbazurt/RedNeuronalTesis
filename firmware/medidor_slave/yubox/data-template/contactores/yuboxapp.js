function setupContactoresTab()
{
    var pane = getYuboxPane('contactores');

    pane.find('form#tempoffset input[name=enabled]').change(function() {
        if (pane.find('form#tempoffset input[name=enabled]:checked').val() == '1') {
            pane.find('form#tempoffset input#sp_fp').prop('disabled', false);

            pane.find('form#activate_rele input[name=enabled_releA]').prop('disabled', true);
            pane.find('form#activate_rele input[name=enabled_releB]').prop('disabled', true);
            pane.find('form#activate_rele input[name=enabled_releC]').prop('disabled', true);

        } else {
            pane.find('form#tempoffset input#sp_fp').prop('disabled', true);

            pane.find('form#activate_rele input[name=enabled_releA]').prop('disabled', false);
            pane.find('form#activate_rele input[name=enabled_releB]').prop('disabled', false);
            pane.find('form#activate_rele input[name=enabled_releC]').prop('disabled', false);
        }
    });

    pane.find('form#tempoffset button[name=saveconfig]').click(function () {
        var postData = {
            enabled:    pane.find('form#tempoffset input[name=enabled]:checked').val(),
            sp_fp:     pane.find('form#tempoffset input[name=sp_fp]').val(),
        };
        $.post(yuboxAPI('contactores')+'/tempoffset.json', postData)
        .done(function (r) {
            yuboxMostrarAlertText('success', r.msg, 3000);
        })
        .fail(function (e) { yuboxStdAjaxFailHandler(e); });
    });


    pane.find('form#activate_rele button[name=saveconfig]').click(function () {
        var postData = {
            releA:    pane.find('form#activate_rele input[name=enabled_releA]:checked').val(),
            releB:    pane.find('form#activate_rele input[name=enabled_releB]:checked').val(),
            releC:    pane.find('form#activate_rele input[name=enabled_releC]:checked').val(),
        };
        $.post(yuboxAPI('contactores')+'/releSet.json', postData)
        .done(function (r) {
            yuboxMostrarAlertText('success', r.msg, 3000);
        })
        .fail(function (e) { yuboxStdAjaxFailHandler(e); });
    });


    // https://getbootstrap.com/docs/4.4/components/navs/#events
    getYuboxNavTab('contactores')
    .on('shown.bs.tab', function (e) {
        yuboxLoadCalibration();
        yuboxLoadAutomatico();
    })
    .on('hide.bs.tab', function (e) {
    });
}

function yuboxLoadCalibration()
{
    $.get(yuboxAPI('contactores')+'/tempoffset.json')
    .done(function (data) {
        var pane = getYuboxPane('contactores');
        pane.find('form#tempoffset input#'+(data.enabled ? 'enabled_on' : 'enabled_off')).click();
        pane.find('form#tempoffset input#sp_fp').val(data.sp_fp);
    })
    .fail(function (e) {
        yuboxStdAjaxFailHandler(e, 3000);
    });
}

function yuboxLoadAutomatico()
{
    $.get(yuboxAPI('contactores')+'/releSet.json')
    .done(function (data) {
        var pane = getYuboxPane('contactores');
        pane.find('form#activate_rele input#'+(data.enabled_faseA ? 'on_releA' : 'off_releA')).click();
        pane.find('form#activate_rele input#'+(data.enabled_faseB ? 'on_releB' : 'off_releB')).click();
        pane.find('form#activate_rele input#'+(data.enabled_faseC ? 'on_releC' : 'off_releC')).click();
    })
    .fail(function (e) {
        yuboxStdAjaxFailHandler(e, 3000);
    });
}
