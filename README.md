# Dokumentacja programu
## Implementacja   
Program został napisany w języku programowania C++, API graficznemu OpenGL w wersji 4.x. Do tworzenia okna, przechwytywania zdarzeń z klawiatury i myszki wykorzystano darmową i unowocześnioną implementację biblioteki glut, czyli freeglut. Program jest napisany z wykorzystaniem IDE Visual Studio 2017, debugowany z użyciem narzędzi dostarczonych wraz z Visual Studio i kompilowany z użyciem dostarczonych z Visual Studio kompilatorów. Projekt jest kompilowany na architekturę procesora x86, z zainstalowanym systemem operacyjnym Windows 8 lub nowszym.   

## Zalecana konfiguracja sprzętowa 
 - CPU: Intel Core i9-7900X
 - RAM: 32GB DDR4
 - GPU: Nvidia Titan V   

## Opis działania
Program wyświetla podwodną scenę 3d z którą możemy wchodzić w interakcję poprzez kierowanie łodzią podwodną która wskazuje skierowanie kamery. Scena składa się z kilku obiektów, mianowicie dna zbiornika wodnego wymodelowanego w zewnętrznym programie i teksturowanego proceduralnie, jednej statycznej łodzi podwodnej stanowiącej punkt centralny sceny, jednej łodzi podwodnej która jest przyczepiona do kamery, oraz ławicy ryb. Wszystkie modele są odpowiednio oświetlone, jeżeli nie są statyczne to zmienia się oświetlenie poszczególnych powierzchni w zależności od położenia. Wszystkie modele posiadają tektury, wykonane dynamicznie lub gotowe w formie pliku png. Dodatkowo dla całej sceny działa wygładzanie krawędzi za pomocą MSAA z próbkowaniem 8, działa również backface culling, co zwiększa wydajność całej aplikacji.   

## Organizacja projektu
Folder `textures` zawiera tekstury dla niektórych obiektów zapisane w formacie *.png* o wielkościach około 1024x1024.   
Folder `models` zawiera pliki modeli które są wykorzystywane w scenie, w formacie Wavefront *.obj*.   
Folder `src` zawiera pliki źródłowe programu.   
Folder `dependencies` zawiera zależności potrzebne do skompilowania programu.   
Folder `shaders` zawiera shadery, polecenia dla karty graficznej odnośnie manipulacji pikseli.   
