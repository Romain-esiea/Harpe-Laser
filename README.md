# Harpe Laser

## Projet Scientifique et Technique : Harpe Laser

### Configuration

1. Connectez la carte Arduino à un ordinateur contenant le code Arduino.
2. Téléversez l'un des deux programmes disponibles :

 - [arduino_harpe-laser_numerique.ino](arduino_harpe-laser_numerique.ino) : produit un son plus puissant, mais au rendu plus numérique.
 - [arduino_harpe-laser_corde.ino](arduino_harpe-laser_corde.ino) : produit un son plus proche de celui d'une harpe, avec une décroissance progressive de la note.
3. Une fois le programme téléversé, attendez la phase de calibration des photorésistances. Pendant cette étape, veillez à ne pas obstruer les faisceaux lumineux.

Les notes de musique par défaut sont : **DO, RÉ, MI, FA, SOL**.

Il est possible de modifier ces notes dans le fichier **pitches.h**.

### Utilisation

Un son est joué lorsque vous relâchez une corde laser. Plus le faisceau est interrompu longtemps, plus la note jouée sera longue :

* Durée minimale : **300 à 500 ms**
* Durée maximale : **2000 ms**

Lorsque plusieurs notes sont jouées simultanément, vous entendrez une fréquence correspondant à la moyenne des notes jouées.

### Avertissement ⚠️

Ne regardez jamais directement les faisceaux laser afin d'éviter tout risque de blessure aux yeux.
