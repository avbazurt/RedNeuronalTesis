% Tesis Medidor Consumo Energetico - Mejorar Factor Potencia

%Vidal Bazurto
%Daniel Guznay

%% Conversiones
% 1 Segundo -> 0.001
% 10 Segundos -> 0.01

%Equivale a un segundo
gain_time = 0.0001;

%Equivale a 10 segundos
ts_save = gain_time *10; 

%Equivalente a 12Horas -> 6AM a 8PM
horas_simular = 0.5;

total_sim =  (horas_simular*60*60)*gain_time

%% Iniciamos variables importantes

%Variables de envio
ts_send = 0.01
ASCII_send = '%f,%f,%f,%f,%f,\n' 

ts_read = 0.05
ASCII_read = '%f|%f|%f\n'

%% Datos Carga Trifasica

data = []

for i = 1:4
    %day
    day = 9 + i

    %weekday
    weekday = 1 +i

    %Seed Ruido Solar
    speed_solar = [round(rand()*100000)]

    %Luz solar 
    duty_solar = (4*rand())-(3*rand())+90

    %Datos de carga RL balanceada
    R=10;

    %Asumir que T=0 es las 6am
    time_hour = 60*60*gain_time;

    T1= 2 *time_hour + rand()*0.5; % 8am
    T2= 6 *time_hour + rand()*0.5; % 12pm
    T3= 11 *time_hour + rand()*0.5; % 5pm
    T4= 17 * time_hour + rand()*0.5; % 0am del día siguiente
    T5=18; % 6am del día siguiente
    L1=8.5e-03 + rand()*1e-04; %se activa de 0-T1
    L2=12e-03 + rand()*1e-04; %se activa de T1-T2
    L3=19e-03 + rand()*1e-04; %se activa de T2-T3
    L4=18e-03 + rand()*1e-04; %se activa de T3-T4
    L5=12e-03 + rand()*1e-04; %se activa de T4-T5


    msg = 'Iniciando Simulacion'
    sim('Planta_FTV_load_balanceada.slx')
    msg = 'Terminando Simulacion'

    data = [data;dataSet]
    
    t = 'Terminado guardado'
    
end

Tabla = array2table(data,'VariableNames',{'day','weekday','second_hora','VA','IA','FPA','VB','IB','FPB','VC','IC','FPC','FP3'})
writetable(Tabla,'TrainModel.csv');



 