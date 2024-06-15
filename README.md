# RT Renderer
**To view this file in English, go [here](README.en.md).**

Renderer s mogućnošću renderiranja pomoću programski implementiranih tehnika praćenje zrake (Ray Tracing) i
praćenje puta Monte Carlovom metodom (Monte Carlo Path Tracing).

**Ova aplikacija je razvijena u kao programski dio završnog rada preddiplomskog studija na
Fakultetu elektrotehnike i računarstva na temu "Postupak praćenja zrake i praćenja puta".**

<figure>
  <img src="examples/pathtrace.png" alt="screenshot" style="width:300px;">
  <figcaption>Primjer slike renderiranje praćenjem puta</figcaption>
</figure>

## Tehnologije
Renderer je napisan u programskom jeziku C++ i OpenGL-u.

## Pokretanje
Pokreniti `run.sh`.

## Upute za korištenje
Pri pokretanju, renderer će se pokrenuti u rasterizer načinu (Phongovo osvjetljenje, onemogućene mape sjena).
Moguće je pomicati se po preddefiniranoj sceni.

Za pokretanje renderiranja pomoću praćenja zrake, potrebno je pritisniti na gumb "2".
Slika će se prikazati na zaslonu čim završi renderiranje.

Moguće se vratiti na rasterizer način pritiskom na gumb "1".
Za pokretanje renderiranja pomoću praćenja puta, potrebno je pritisniti na gumb "3",
to će napraviti jednu iteraciju praćenja puta.

Ostale mogućnosti:
- možemo uključiti automatsko renderiranje i Monte Carlo integraciju pritiskom na gumb "E"
- možemo povećati dubinu renderiranja pritiskom na strelicu gore ili dolje
- možemo povećavati ili smanjivati koeficijent zrcaljenja pritiskom na strelice lijevo i desno
- možemo mijenjati hrapavost površine pritiskom na gumbe stranica gore i dolje (vrijedi samo za algoritam praćenja puta).
