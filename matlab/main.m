% Tesis Medidor Consumo Energetico - Mejorar Factor Potencia

%Vidal Bazurto
%Daniel Guznay

%% Conversiones
clear 
clc
% 1 Segundo -> 0.001
% 10 Segundos -> 0.01

%Equivale a un segundo
gain_time = 0.0001;

%Equivale a 10 segundos
ts_save = gain_time *10; 

%Equivalente a 12Horas -> 6AM a 8PM
horas_simular = 14;


hora_inicio = 6*60*60*gain_time;
ocultarSol = 12*60*60*gain_time; %5PM

total_sim =  (horas_simular*60*60)*gain_time

%% Iniciamos variables importantes

%Variables de envio
ts_send = 0.01
ASCII_send = '%f,%f,%f,%f,%f,\n' 

ts_read = 0.01
ASCII_read = '%f|%f|%f\n'

%% Datos Carga Trifasica
clc
data = []

%weekday
weekday = 1;

for dia = 1:2

    day = dia;
    
    if (weekday>7)
        weekday = 1
    else
        weekday = weekday +1
    end
    

    %Seed Ruido Solar
    speed_solar = [round(rand()*100000)]

    %Luz solar 
    duty_solar = (4*rand())-(3*rand())+90

    %Datos de carga RL balanceada
    R=10;

    %Asumir que T=0 es las 6am
    time_hour = 60*60*gain_time;

    
    if (weekday<6)
        T1= 0.8 *time_hour + rand()*time_hour ; % 8am
        T2= 3 *time_hour + rand()*time_hour; % 9am
        T3= 5 *time_hour + rand()*time_hour;% 11am
        T4= 7 * time_hour + rand()*time_hour ;% 1pm
        T5= 10 * time_hour + rand()*time_hour;% 4pm
        T6 = 10; % 6am del día siguiente
        

        L1=6.5e-03 + rand()*1e-04; %se activa de 0-T1
        L2=12e-03 + rand()*1e-04; %se activa de T1-T2
        L3=19e-03 + rand()*1e-04; %se activa de T2-T3
        L4=18e-03 + rand()*1e-04; %se activa de T3-T4
        L5=12e-03 + rand()*1e-04; %se activa de T4-T5
        L6=13e-03+ rand()*1e-04;%se activa de T5-T6
    else

        T1= 3*time_hour + rand()*time_hour; % 9 am
        T2= 4*time_hour + rand()*time_hour; % 10 am
        T3= 6*time_hour + rand()*time_hour; % 12pm
        T4= 9*time_hour + rand()*time_hour; % 3pm
        T5= 11*time_hour + rand()*time_hour; % 5pm
        T6= 11.5*time_hour + rand()*time_hour; % 5:30pm del día siguiente
        
  
        T1R = 10;
        T2R = 10;
        L1=10e-03+ rand()*1e-04; %se activa de 0-T1
        L2=9.5e-03+ rand()*1e-04; %se activa de T1-T2
        L3=15e-03+ rand()*1e-04; %se activa de T2-T3
        L4=11e-03+ rand()*1e-04; %se activa de T3-T4
        L5=17e-03+ rand()*1e-04; %se activa de T4-T5
        L6=13e-03+ rand()*1e-04;%se activa de T5-T6
    end
    
   
    msg = 'Iniciando Simulacion'
    sim('Planta_FTV_load_balanceada.slx')
    msg = 'Terminando Simulacion'

    data = [data;dataSet]
    
    t = 'Terminado guardado'
    
end

%%
Tabla = array2table(data,'VariableNames',{'day','weekday','second_hora','VA','IA','FPA','VB','IB','FPB','VC','IC','FPC','FP3'})
writetable(Tabla,'TrainModel.csv');

%% Conectar SQL DataBase
msg = 'Comenzar'
conn = database('RASPBERRY PI','avbazurt','');
sqlwrite(conn,"mediciones",Tabla)
msg = 'Listo'

%% Caracteristicas para test
day = 1
weekday = 1

%Seed Ruido Solar
speed_solar = [round(rand()*100000)]

%Luz solar 
duty_solar = (4*rand())-(3*rand())+90

%Datos de carga RL balanceada
R=10;

%Asumir que T=0 es las 6am
time_hour = 60*60*gain_time;

T1= 0.8 *time_hour ; % 8am
T2= 3 *time_hour + rand()*time_hour; % 9am
T3= 5 *time_hour + rand()*time_hour;% 11am
T4= 7 * time_hour + rand()*time_hour ;% 1pm
T5= 10 * time_hour + rand()*time_hour;% 4pm
T6 = 10; % 6am del día siguiente


L1=6.5e-03 + rand()*1e-04; %se activa de 0-T1
L2=12e-03 + rand()*1e-04; %se activa de T1-T2
L3=19e-03 + rand()*1e-04; %se activa de T2-T3
L4=18e-03 + rand()*1e-04; %se activa de T3-T4
L5=12e-03 + rand()*1e-04; %se activa de T4-T5
L6=13e-03+ rand()*1e-04;%se activa de T5-T6


C1 = 60e-6

 