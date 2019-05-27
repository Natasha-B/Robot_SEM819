# Carte Master

  La carte Master reçoit les ordres de la centrale de commande et est chargée de les transmettre à différents périphériques.
  
  Elle traduit les ordres de la centrale de commande pour la carte Serializer qui gère les moteurs de déplacement de la base roulante. 
  Elle transmet aussi des ordres au servomoteur du télémètre qui fait tourner ce dernier d'un angle défini avant d'effectuer une mesure de distance. Le résultat de cette mesure sera renvoyé à la centrale de commande. 
  Enfin, elle doit être capable d'effectuer une mesure du courant consommé et d'enoyer des informations via liaison série à la carte slave. 
  
  
## Liste des commandes envoyées par la centrale:



Commande                     |        Signification
-----------------------------|--------------------------------------
  Q                          |          arrêt d'urgence
  A [vitesse]                |      avancer à la vitesse [v]
  B [vitesse]                |      reculer à la vitesse [v]
  S                          |              arrêt
  RD                         |        rotation 90° droite
  RG                         |        rotation 90° gauche
  RC                         |          rotation de 180°
  E                          |        début de l'épreuve
  D [numéro d'épreuve]       |       début épreuve [numéro]
  TV [vitesse]               | définition de la vitesse par défaut




Exemples de commande:
     
     TV 20 A        # Avance à la vitesse 20
     TV 20 B 10     # Recule à la vitesse 10
     TV 20 RD       # Tourne à droite 
     
