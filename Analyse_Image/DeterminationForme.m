function Forme = DeterminationForme(imagette_col, Seuil_down)
  
  
  %Seuillage de l'imagette
  imagette_seuil = im2double(im2bw(rgb2gray(imagette_col), Seuil_down));
  imagette_seuil = imerode(imagette_seuil, [0,1,0;1,1,1;0,1,0]);
  [hi,wi] = size(imagette_seuil);
  %Détermination du gradient
  [Gx,Gy] = gradient(imagette_seuil);
  %Détermination de la norme du gradient
  NG = Gx.^2 + Gy.^2;
  %Détection de droites
  [Hint,thetaint,Rhoint] = hough(NG,'Thetaresolution', 1);
  indicemaxmax = find(Hint == max(max(Hint)));
  if length(thetaint(ceil(indicemaxmax/length(Rhoint)))) > 1
    Theta = thetaint(ceil(indicemaxmax/length(Rhoint)));
    Theta = Theta(1)
  else
      Theta = thetaint(ceil(indicemaxmax/length(Rhoint)))
  end
  while (Theta(1) - 5 >= -90)
      Theta = [Theta(1) - 5 , Theta];
  end
  while (Theta(end)+5 <= 89)
      Theta = [Theta, Theta(end)+5];
  end
  [H,theta,Rho] = hough(NG,'Theta', Theta);
  
  %Seuillage
  H_seuil = (H > max(max(H))/2).*H;

  Maximas_H = imregionalmax(H_seuil);
  %Le nombre de maximas permet de déterminer les formes (ou un traitement plus approfondi)
  Indices_maxi = find(Maximas_H);
  L_indices = length(Indices_maxi)

  if (L_indices == 4) %Carre
    Forme = 'Carre';
  elseif (L_indices == 3) %Triangle
    Forme = 'Triangle';
  elseif ((L_indices <= 8) && (L_indices >= 5)) %Croix ou carre_creux
    
    if imagette_seuil(floor(hi/2),floor(wi/2)) == 1  %Carre_creux
      Forme = 'Carre_creux';
    else    %Croix
      Forme = 'Croix';  
    end
  else %Rond ou Coeur
    [C,R,M] = imfindcircles(im2bw(NG,0.01),[ceil(length(NG)/15) ceil(length(NG)/3)],'SENSITIVITY',0.9);
    nombre = length(R);
    
    if nombre == 1    %Rond
      Forme = 'Rond';
    else    %Coeur
      Forme = 'Coeur';  
    end
    end
    
    
%   figure(6)
%   subplot(221)
%   imshow(imagette_seuil)
%   title('Imagette seuillee pour determination de la forme')
% 
%   subplot(222)
%   imshow(NG,[]);
%   title('Norme du gradient de l''image');
% 
%   subplot(223)
%   imshow(Gx,[]);
%   title('GRadient suivant x de l''image');
% 
%   subplot(224)
%   title('Gradient suivant y de l''image');
%   imshow(Gy,[]);
% 
%   figure(7)
%   subplot(221);
%   imshow(Maximas_H,[]);
%   title('Matrice d''accumlation detection de droites')
  end

