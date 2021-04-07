### ARM-KL05: LED náramkové hodinky na bázi RTC modulu  
Odkaz na video: https://nextcloud.fit.vutbr.cz/s/7dLBJtZDPtboj83
## Zadání/účel projektu  
Úkolem bylo naprogramovat mikro-kontrolér KL05 na vývojové desce se čtyřmi sedmi
segmentovými led displeji a tlačítkem. Deska byla poskytnuta fakultou/cvičícím. Cílem byla
funkcionalita náramkových hodinek, kde je čas zobrazován pouze po stisknutí tlačítka. Dále
mají být implementovány postupy pro šetření energie.  
## Spuštění a vývojové prostředí
V odevzdaném archivu je celá složka projektu s tím, že kromě souboru main.c jsou veškeré
soubory ve stavu, v jakém byli vygenerovány vývojovém prostředím. Tímto prostředím
Kinetis studio 3 poskytnuté v zadání.
Projekt tedy stačí přeložit a nahrát do kitu. Po necelé minutě budou hodinky spuštěny a plně
fungovat.
## Implementace  
Implementace je založena na na programu z display_test.zip, který zajistil základní práci s
displajem, tlačítkem a inicializaci mcu. Iluze rozsvícení všech displejů najednou je dosaženo
střídavým rozsvícením a zhasínáním displejů dostatečně rychle za sebou. Pro potřebu
nastavování a zobrazení času jsou použity stavy “DEFAULT, MINUTES, HOURS”. Stavy
jsou nastavovány v PORTB_IRQHandler, která zajišťuje přerušení vygenerované stiskem
tlačítka. Stav se mění podržením tlačítka (2 sekundy) a mění funkci, kterou má stisknutí
tlačítka.  
Pro získání času je použít RTC modul, který je inicializován funkcí RTCInit. Je použít pro
generování přerušení každých 60 sekund.  
Ve výchozím režimu stisknutí tlačítka zobrazí čas. Podržením se přepne režim na
nastavování minut, kde se přidává minuta opět stisknutím tlačítka. Dalším podržením se
přepne režim na nastavení hodin, kde se hodiny přidávají ve stejném duchu, stisknutím
tlačítka. Dalším podržením se vrací do výchozího režimu.  
## Závěr  
Řešení pokrylo veškerou funkcionalitu pro zobrazování a nastavování času, kde k počítání
času byl využíván RTC modul. Chybí implementace uspávání MCU, případně jiných postupu
pro snížení spotřeby
