# RTOS
Kurssilla opettelemme sulautetun järjestelmän ohjelmoimointia mm. useilla thredeillä ja miten puskuria tulisi käyttää datan tallennukseen.

### Kurssi jakaantuu sisäisesti kahteen asiaan: 
- RTOS-ohjelmointi
- Testivetoinen ohjelmistokehitys

### Ohjelmointityökalut
- Sulautettu ohjelmistokehitys: C++ ja Zephyr RTOS
- Kurssin laitteet vaatii nRF Connect SDK:n asennuksen
- Visual Studio Codea käytetään ohjelmointiympäristönä
- nRF Connect-laajennukset löytyy codelle valmiina

### Testiautomaatio
 - Google Test yksikkötestaukseen
 - Robot Framework testiautomaatioon

## Demo video:
Videossa on demonstroitu miten ledejä ohjataan UARTIN kautta tulevalla datalla, käyttäen thredejä ja puskuria. 

käytettävissä olevat komennot:

Esim. 

- "r5000g5000y5000" -> määrätään sekvenssi eri värisiä ledejä ja niiden kesto millisekunteina.
- "d" -> debug printtaukset ON/OFF.
- "000003" -> ajastinkeskeytys, sleeppaa aluksi annetun ajan, (tässä tapauksessa 3s) jonka jälkeen punainen ledi menee päälle 3s ajaksi. 



[![RTOS Demo](https://img.youtube.com/vi/o8tv2ko1EEo/0.jpg)](https://youtu.be/o8tv2ko1EEo)
