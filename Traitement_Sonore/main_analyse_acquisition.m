%% Traitement des signaux sonores
%{
Les balises émettent des sons. Cette fonction de traitement doit permettre
d'identifier si la balise émet un son.
Dans ce cas là, on analyse le son émis pour déterminer sa fréquence F, ses
durées d'émission dP1 et de silence dW1.

Entrée: réception du signal

Sortie: Fichier contenant F, dP1 et dW1


        !!  FICHIER FINAL  !!
%}
%% Chargement du signal
%{clear all; close all;

[sig,Fe]=audioread('Sinus_440Hz_4X_1s_1s_.wav');
%longueur du signal
l=length(sig);
%Création du vecteur temps
t=(1:l)./Fe;
t=t'; %Passage en vec colonne comme sig
%}

%% Extraction signal
Fichier=fopen('cortex.txt', 'r');
data=textscan(Fichier, '%s');
% Char par 2
% str2num -> code ascii-48
%X1*64+x2 
%Freq echatillon 5128.205

for  a=1:1:length(data{1,1})
ligne=data{1,1}{a,:};

for i=1:2:length(ligne)-1
    sig_str(1,i)=(ligne(1,i)-48)+64*(ligne(1,i+1)-48);
     
end

end
sig=sig_str;
Fe=5128;
%longueur du signal
l=length(sig);
%Création du vecteur temps
t=(1:l)./Fe;
t=t'; %Passage en vec colonne comme sig

figure(6)
plot(t,sig);
title('signal');
%% Estimation de la fréquence 

NFFT = 2^nextpow2(l); % Next power of 2 from length of y
Y = fft(sig,NFFT)/l;
f = Fe/2*linspace(0,1,NFFT/2+1);

%Spectre des fréquences
figure(3)
plot(f,2*abs(Y(1:NFFT/2+1))) ;
title('Spectre des fréquences');
xlabel('Fréquences (Hz)');
ylabel('|Y(f)|');

%Recherche du maximum
bruit=1000;
spectre=2*abs(Y(2:NFFT/2+1));
index=find(spectre<bruit);
debruit=spectre( index(1,:));
debruit=debruit';
[C,I]=max(debruit);
Frequence_signal = debruit(I,:)

%% Analyse a=a+1;du signal re�u
%Enveloppe du signal
[sig_up, sig_low]=envelope(debruit,100,'rms');
figure(2)
hold on;
ld=length(debruit);
td=(1:ld)./Fe;
td=td';

plot(td, sig_up, td,sig_low);
title('Enveloppe du signal');

%% Temps d'émission

% Identification des temps d'emission
Seuil= (max(sig_up)+min(sig_up))/2;
Resultat = sig_up > Seuil;
Decale = [Resultat(2:end); Resultat(end)];

Coupure = Resultat ~= Decale;
figure(5)
plot(td, Coupure, td, Resultat)

%Estimation de la durée
Indices=find(Coupure);
for i=1:1:length(Indices)-1
    Durees(i)=Indices(i+1,:)-Indices(i,:); 
      
end

Em=(Durees(1)+Durees(3))/2;
Si=(Durees(2)+Durees(4))/2;

dP1=Em/Fe;
dW1=Si/Fe;

%% Ecriture dans fichier
fichier=fopen('donnees.txt','wt');
fprintf(fichier,' %d, %d, %d', Frequence_signal, dW1,dP1);
