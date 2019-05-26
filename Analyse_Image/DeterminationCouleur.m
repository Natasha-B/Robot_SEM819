function couleur = DeterminationCouleur(imagette_coul)
  %Détermination de la référence en HSV
  image_ref = [[0.5,0,0.5];[1,0,1];[0,1,0];[1,1,0];[1,0.5,0];[0, 1, 1]];
  image_ref1 = zeros(6,1,3);
  image_ref1(:,:,1) = image_ref(:,1);
  image_ref1(:,:,2) = image_ref(:,2);
  image_ref1(:,:,3) = image_ref(:,3);
  image_ref_hsv = rgb2hsv(image_ref1);
  
  %Conversion en HSV
  imagette_HSV = rgb2hsv(imagette_coul);
  [hh,wh,nh] = size(imagette_HSV);
  
  %Calcul de la teinte moyenne et de la "valeur" moyenne
  im_H = imagette_HSV(:,:,1);
  im_V = imagette_HSV(:,:,3);
  Hmoy = 0;
  Vmoy = 0;
  n = 0;
  for indice = 1:hh*wh
    if im_H(indice) ~= 0
      Hmoy = Hmoy + im_H(indice);
      Vmoy = Vmoy + im_V(indice);
      n = n + 1;
    end
  end
  Hmoy = Hmoy/n;
  Vmoy = Hmoy/n;
  
  %Recherche de la couleur correspondante en se basant sur la teinte
  comp = abs(image_ref_hsv(:,:,1) - Hmoy);
  mini = min(comp); 
  meilleur = find((comp == mini));
  
  %Test si l'on se trouve sur des teintes de rose ou violet
  if length(meilleur) > 1
    %Recherche de la couleur correspondante en se basant sur la "valeur"
    comp = abs(image_ref_hsv(1:2,:,3) - Vmoy);
    mini = min(comp);
    meilleur = find((comp == mini));
  end
    
  switch meilleur
    case 1 
      couleur = 'Violet'; 
    case 2
      couleur = 'Magenta';
    case 3
      couleur = 'Vert';
    case 4
      couleur = 'Jaune';
    case 5
      couleur = 'Orange';
    case 6
      couleur = 'Cyan';
      otherwise
          couleur = ' ';
  end
  
%   figure(5)
%   subplot(222)
%   imshow(imagette_coul)
%   title('imagette de la couleur a determiner')
  
end
