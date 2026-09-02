# Maršrutizatoriaus duomenų nuskaitymas ir komunikacija su debesų platforma
Darbo tikslas – paruošti programą, kuri siųstų duomenis apie maršrutizatorių į Tuya IoT debesų platformą, bei paruošti jai, bei naudojamoms Tuya IoT SDK bibliotekoms, OpenWRT paketus. Taip pat paruošti minimalų API endpoint‘ą.

Darbo užduotys: 
1)	Paruošti OpenWRT paketą daemon tipo programai, kuri siųstų routerio sistemos informaciją į Tuya IoT debesų platformą. Duomenims apie routerį gauti naudoti UBUS sistemą, bei duomenis apie programos veikimą įrašyti į sistemos įvykių žurnalą (angl. „log“).
2)	Paruošti OpenWRT paketą bibliotekai/SDK, kuria naudojantis bus komunikuojama su Tuya IoT debesų platforma. Bibliotekos kodą automatiškai atsiųsti iš nuotolinio serverio.
3)	Paruošti minimalų API endpoint‘ą programai valdyti.
