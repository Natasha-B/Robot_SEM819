# Système d'annalyse d'images 

## Le programme s'organise en différentes sous fonctions : 

- Le programme Analyse_image.m  : Il s'agit ici du programme principal qui va permettre à l'ensemble des fonctions suivantes de s'organiser correctement. 
                                L'utilisateur ne devrait aveoir qu'à manipuler celui ci afin de changer le chemin d'accès, changer le chemin de sortie, rien de plus.

- La fonction Seuillage3niveaux.m : cette fonction permet de seuiller l'image sur trois zones distinctes : les valeurs faibles seront nulles, les valeurs milieues seront grises et les valeurs fortes seront blanches.

- La fonction RechercheForme.m : cette fonction permet de rechercher soit la couleur soit la forme en elle même en utilisant une reconstruction morphologique

- La fonction ExtractionImagette.m : cette fonction permet d'extraire une nouvelle image contenant uniquement les données de couleurs et de forme interessantes

- La fonction DéterminationCouleur.m : cette fonction permet de déterminer la couleur de l'image à l'aide d'un passage dans l'espace HSV

- La fonction DéterminationForme.m : cette fonction permet de déterminer la forme de l'objet à l'aide de différentes transformées de Hough


Les seules références à notifiée concernant ce code réside dans le cours dispensé en 4ETI de la majeure IMI.
