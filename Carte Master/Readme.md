# Carte Master

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
     
## Liste des commandes envoyées par la carte master à la serializer:
