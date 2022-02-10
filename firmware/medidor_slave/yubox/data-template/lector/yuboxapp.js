function setupEnergyGraphicsTab()
{
    var lectorpane = getYuboxPane('lector');
    var data = {
        'sse':  null,
        'chart': null,
    };
    lectorpane.data(data);

    var initData = {
        power:  {
            title:  'Potencia',
            canvas: 'canvas#canvas_power',
            datasets: [
                {
                    label:      'P3Φ. Real (W)',
                    borderColor:'rgb(255, 99, 132)',
                    yAxisID:    'y-axis-id',
                },{
                    label:      'P3Φ. Reactiva (VAR)',
                    borderColor:'rgb(54, 162, 235)',
                    yAxisID:    'y-axis-id',
                },{
                    label:      'P3Φ. Aparente (VAR)',
                    borderColor:'rgb(162, 235, 54)',
                    yAxisID:    'y-axis-id',
                }
            ]
        },


        power_factor_tri: {
            title:  'Factor de potencia Trifasico',
            canvas: 'canvas#canvas_power_factor_tri',
            datasets: [
                {
                    label:      'Factor Potencia Trifasico',
                    borderColor:'rgb(128, 0, 255)',
                    yAxisID:    'y-axis-id',
                }
            ]
        },

        //LOS COLORES SIGUEN NORMATIVA NEC - 120 208V
        power_factor: {
            title:  'Factor de potencia',
            canvas: 'canvas#canvas_power_factor',
            datasets: [
                {
                    label:      'Fase A',
                    borderColor:'rgb(0, 0, 0)',
                    yAxisID:    'y-axis-id',
                },
                {
                    label:      'Fase B',
                    borderColor:'rgb(255, 0, 0)',
                    yAxisID:    'y-axis-id',
                },
                {
                    label:      'Fase C',
                    borderColor:'rgb(0, 255, 0)',
                    yAxisID:    'y-axis-id',
                }
            ]
        },
        current: {
            title:  'Corriente',
            canvas: 'canvas#canvas_current',
            datasets: [
                {
                    label:      'Fase A',
                    borderColor:'rgb(0, 0, 0)',
                    yAxisID:    'y-axis-id',
                },
                {
                    label:      'Fase B',
                    borderColor:'rgb(255, 0, 0)',
                    yAxisID:    'y-axis-id',
                },
                {
                    label:      'Fase C',
                    borderColor:'rgb(0, 255, 0)',
                    yAxisID:    'y-axis-id',
                }
            ]
        },
        voltage: {
            title:  'Voltaje',
            canvas: 'canvas#canvas_voltage',
            datasets: [
                {
                    label:      'Fase A',
                    borderColor:'rgb(0, 0, 0)',
                    yAxisID:    'y-axis-id',
                },
                {
                    label:      'Fase B',
                    borderColor:'rgb(255, 0, 0)',
                    yAxisID:    'y-axis-id',
                },
                {
                    label:      'Fase C',
                    borderColor:'rgb(0, 255, 0)',
                    yAxisID:    'y-axis-id',
                }
            ]
        }
    };

    var chts = {};
    for (var k in initData) {
        var ctx = lectorpane.find(initData[k].canvas)[0].getContext('2d');
        var chtData = {
            datasets: initData[k].datasets
        };
        for (var i = 0; i < chtData.datasets.length; i++) {
            chtData.datasets[i].backgroundColor = chtData.datasets[i].borderColor;
            chtData.datasets[i].fill = false;
            chtData.datasets[i].data = [];
        }
        var chtOpt = {
            type:       'line',
            data:       chtData,
            options:    {
                responsive: true,
                hoverMode: 'index',
                stacked: false,
                title: {
                    display: true,
                    text: initData[k].title
                },
                scales: {
                    xAxes:  [{
                        type:   'time'
                    }],
                    yAxes:  [{
                        type:       'linear',
                        display:    true,
                        position:   'left',
                        id:         'y-axis-id',
                        ticks:  {
                            beginAtZero: true
                        }
                    }]
                }
            }
        };

        chts[k] = new Chart(ctx, chtOpt);
    }

    lectorpane.data('chart', chts);

    if (!!window.EventSource) {
        var sse = new EventSource(yuboxAPI('lectura')+'/events');
        sse.addEventListener('EnergyReport', function (e) {
            var data = $.parseJSON(e.data);
            yuboxLector_actualizar(data);
        });
        lectorpane.data('sse', sse);
    }
}

function yuboxLector_actualizar(data)
{
    var d = (data.timestamp != null) ? new Date(data.timestamp * 1000) : new Date();

    var text = "some text"
    console.log(text)

    var lectorpane = getYuboxPane('lector');
    var chts = lectorpane.data('chart');
    
    console.log(data);

    for (var k in chts) {
        var samp = [];

        // Preparar y empujar muestra según la clave
        switch (k) {
        case 'power':
            samp.push({x: d, y: data.phases[0].real_power});
            samp.push({x: d, y: data.phases[0].reactive_power});
            samp.push({x: d, y: data.phases[0].apparent_power});
            lectorpane.find('h3#real_power').text(data.phases[0].real_power.toFixed(2));
            lectorpane.find('h3#reactive_power').text(data.phases[0].reactive_power.toFixed(2));
            lectorpane.find('h3#apparent_power').text(data.phases[0].apparent_power.toFixed(2));
            break;

        case 'power_factor_tri':
            samp.push({x: d, y: data.phases[0].power_factor_tri});
            lectorpane.find('h3#power_factor_tri').text(data.phases[0].power_factor_tri.toFixed(2));
            break;

        case 'power_factor':
            samp.push({x: d, y: data.phases[0].power_factor_a});
            samp.push({x: d, y: data.phases[0].power_factor_b});
            samp.push({x: d, y: data.phases[0].power_factor_c});
            lectorpane.find('h3#power_factor_a').text(data.phases[0].power_factor_a.toFixed(2));
            lectorpane.find('h3#power_factor_b').text(data.phases[0].power_factor_b.toFixed(2));
            lectorpane.find('h3#power_factor_c').text(data.phases[0].power_factor_c.toFixed(2));
            break;
        case 'voltage':
            samp.push({x: d, y: data.phases[0].voltage_a});
            samp.push({x: d, y: data.phases[0].voltage_b});
            samp.push({x: d, y: data.phases[0].voltage_c});
            lectorpane.find('h3#voltage_a').text(data.phases[0].voltage_a.toFixed(2));
            lectorpane.find('h3#voltage_b').text(data.phases[0].voltage_b.toFixed(2));
            lectorpane.find('h3#voltage_c').text(data.phases[0].voltage_c.toFixed(2));

            break;
        case 'current':
            samp.push({x: d, y: data.phases[0].current_a});
            samp.push({x: d, y: data.phases[0].current_b});
            samp.push({x: d, y: data.phases[0].current_c});

            lectorpane.find('h3#current_a').text(data.phases[0].current_a.toFixed(2));
            lectorpane.find('h3#current_b').text(data.phases[0].current_b.toFixed(2));
            lectorpane.find('h3#current_c').text(data.phases[0].current_c.toFixed(2));
            break;
        }
        for (var i = 0; i < chts[k].data.datasets.length; i++) {
            chts[k].data.datasets[i].data.push(samp[i]);
        }

        // No mostrar datos más viejos que 10 minutos
        if (d.valueOf() - chts[k].data.datasets[0].data[0].x.valueOf() > 10 * 60 * 1000) {
            for (var i = 0; i < chts[k].data.datasets.length; i++) {
                chts[k].data.datasets[i].data.shift();
            }
        }
        chts[k].update();
    }
}