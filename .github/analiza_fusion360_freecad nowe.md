
# Analiza systemu modelowania Autodesk Fusion 360 i wnioski projektowe dla FreeCAD

## 1. Cel dokumentu

Ten dokument ma odpowiedzieć na dwa pytania:

1. **Jak naprawdę działa modelowanie w Autodesk Fusion 360 od strony użytkowej i systemowej?**
2. **Jakie konkretne elementy trzeba wprowadzić do FreeCAD, aby modelowanie i nawigacja były odczuwalnie bliższe Fusion 360, ale bez niszczenia mocnych stron FreeCAD?**

Najważniejszy wniosek już na starcie:

> **Fusion 360 nie jest „przyjemny”, bo ma mniej możliwości. Jest przyjemny dlatego, że nad złożonym silnikiem geometrycznym buduje bardzo spójną warstwę interakcji.**  
> Dla moda FreeCAD najważniejsze nie jest kopiowanie ikon, kolorów czy nazw, ale skopiowanie **gramatyki pracy**: wyboru, podglądu, manipulacji, wpisania wartości, potwierdzenia i późniejszej edycji.

Zakres dokumentu obejmuje przede wszystkim **modelowanie w obszarze Design**: szkice, bryły, powierzchnie, formy/SubD, edycję bezpośrednią, parametry, strukturę modelu, selekcję, historię i nawigację.  
Świadomie nie skupiam się na CAM, renderingu, chmurze danych ani dokumentacji rysunkowej.

---

## 2. Streszczenie wykonawcze

Jeżeli celem jest uzyskać w FreeCAD „feeling” zbliżony do Fusion 360, to nie trzeba w pierwszej kolejności przepisywać wszystkich narzędzi. Trzeba zbudować kilka kluczowych warstw:

### 2.1. Co najbardziej odpowiada za przyjemność pracy w Fusion 360

1. **Jeden spójny język operacji**  
   Ekstruzja, sweep, loft, boundary fill i część innych narzędzi używa tej samej logiki rezultatu: **Join / Cut / Intersect / New Body / New Component**. Użytkownik nie uczy się osobno „pad”, „pocket”, „cut extrude”, „new part”; uczy się jednego modelu mentalnego. [A12] [A16]

2. **Mały podatek decyzyjny na wejściu**  
   W wielu sytuacjach użytkownik nie musi najpierw zdecydować, *jakie narzędzie uruchomić*. Wystarczy wskazać geometrię i użyć narzędzia adaptacyjnego (np. Press Pull), które mapuje:
   - profil szkicu -> ekstrudowanie,
   - krawędź -> fillet,
   - ściana -> offset face. [A5]

3. **Historia parametryczna i edycja bezpośrednia współistnieją**  
   Fusion nie zmusza do wyznawania jednej religii CAD. Można pracować z historią, a gdy potrzeba lokalnej „chirurgii” na geometrii albo pracy z importem, można przejść w direct modeling. [A1] [A2] [A23]

4. **UI jest lokalne względem kursora i obiektu**  
   Sketch Palette, manipulator, menu radialne, toolbox pod klawiszem `S` – wszystko to skraca drogę kursora i liczbę przełączeń uwagi. [A6] [A7] [A26]

5. **Selekcja jest traktowana jako osobny system, a nie detal**  
   Filtry typów, priorytety selekcji, wybór obiektów ukrytych „za” innymi, selekcja window/crossing/freeform – to wszystko drastycznie obniża frustrację. [A9] [A10] [A11]

6. **Kontekst aktywnego komponentu porządkuje pracę**  
   Aktywny komponent filtruje historię i wizualnie wygasza rodzeństwo. To zmniejsza liczbę przypadkowych powiązań między częściami. [A1] [A19]

7. **Parametry są wszędzie, a nie w osobnym, „zaawansowanym” świecie**  
   Można tworzyć parametry w tabeli, ale też „w locie” w polach komend; pola liczbowe akceptują wyrażenia i jednostki. [A21] [A22]

8. **Fusion rozdziela strukturę od czasu**  
   Browser pokazuje hierarchię modelu, a Timeline – historię jego powstawania. To fundamentalnie poprawia czytelność. [A1]

9. **Nawigacja jest elementem modelowania, a nie osobnym trybem pracy**  
   Domyślny preset Fusion trzyma pan/zoom/orbit stale „pod ręką” na myszy, a klawiaturę zostawia głównie do wywoływania komend i przełączania elementów interfejsu. Dodatkowo punkt obrotu można lokalnie przejąć gestem `Shift` + klik środkowym przyciskiem, a preferencje pozwalają przełączyć cały zestaw PZO na presety innych CAD-ów. [A26] [A27]

### 2.2. Co trzeba zrobić w FreeCAD, aby uzyskać podobny efekt

Najbardziej opłacalne zmiany to:

- **jedna zunifikowana powłoka modelowania** nad obecnymi workbenchami,
- **in-canvas UI** dla szkicu i poleceń modelowania,
- **adaptacyjne polecenie typu Press Pull**,
- **ujednolicony Extrude/Hole/Pattern/Mirror** z tym samym słownikiem operacji,
- **aktywny kontekst modelowania** (komponent / body / szkic) z filtrowaniem drzewa i historii,
- **lekki pasek historii / timeline dla aktywnego kontekstu**,
- **selektory priorytetu i filtrów**, wybór obiektów ukrytych,
- **Fusion-like preset nawigacji** jako dodatkowy styl, nie jako zamiana wszystkiego,
- **parametry i wyrażenia w każdym polu liczbowym**,
- **zestaw bezpośrednich edycji ścian**: move/offset/delete/replace face.

Jeżeli mod FreeCAD dostarczy te 10 rzeczy, osiągnie dużą część „przyjemności” Fusion 360 nawet bez pełnej zgodności funkcja-funkcja.

---

## 3. Architektura doświadczenia Fusion 360 – pięć warstw

Fusion 360 warto rozumieć nie jako listę komend, lecz jako system pięciu warstw.

| Warstwa | Co obejmuje | Dlaczego jest ważna |
|---|---|---|
| **Interakcja** | selekcja, nawigacja, menu radialne, toolbox, manipulatory, podgląd | To właśnie ta warstwa daje odczucie „lekkości” |
| **Kontekst** | aktywny komponent, aktywne body, aktywny szkic, widoczność uczestników operacji | Ogranicza chaos i przypadkowe relacje |
| **Intencja projektowa** | szkice, ograniczenia, wymiary, parametry, feature’y | To tu użytkownik opisuje „co ma powstać” |
| **Reprezentacja geometrii** | bryły, powierzchnie, T-Spline, mesh, sheet metal | Różne klasy obiektów służą różnym etapom modelowania [A20] |
| **Historia i przeliczenie** | timeline, suppress, reorder, rollback, parametric/direct mode | Pozwala edytować decyzje w czasie [A1] [A2] |

### 3.1. Najważniejsza obserwacja dla FreeCAD

FreeCAD jest dziś bardzo mocny w warstwach:
- **intencji projektowej**,
- **geometrii**,
- **otwartości i skryptowalności**.

Natomiast relatywnie słabszy bywa w:
- **warstwie interakcji**,
- **płynnym prowadzeniu kontekstu**,
- **czytelnej prezentacji historii**,
- **jednolitości UX między workbenchami**.

To jest dobra wiadomość:  
**nie trzeba przebudowywać całego FreeCAD od zera. Trzeba zbudować lepszą warstwę wejścia i prowadzenia użytkownika nad istniejącą bazą.**

---

## 4. Podstawowe funkcje modelowania w Fusion 360 – jak działają naprawdę

W tej sekcji opisuję najważniejsze grupy funkcji nie tylko z perspektywy „co robi przycisk”, ale także:
- **co robi system pod spodem na poziomie workflow**,  
- **dlaczego użytkownik odbiera to jako wygodne**,  
- **co z tego należy przenieść do FreeCAD**.

---

## 5. Szkic jako centrum modelowania

### 5.1. Jak szkic działa w Fusion 360

W Fusion szkic jest podstawowym nośnikiem intencji dla precyzyjnych feature’ów. Tworząc lub edytując szkic, użytkownik dostaje **Sketch Palette w samym obszarze roboczym**, a nie tylko odległy panel boczny. Palette zawiera m.in.:
- orientowanie kamery na płaszczyznę szkicu (`Look At`),
- siatkę i snap do siatki,
- `Slice` – tymczasowe przekrojenie brył przez płaszczyznę szkicu,
- podświetlanie profili zamkniętych,
- pokazywanie punktów, wymiarów, więzów i geometrii rzutowanej,
- przełącznik `3D Sketch`. [A7]

Ważne jest też to, że **szkic tworzony na istniejącej ścianie planarne** może automatycznie odziedziczyć krawędzie tej ściany jako geometrię rzutowaną. Sama komenda `Project` może utrzymywać lub zrywać asocjację z geometrią źródłową. [A8]

Do tego dochodzi:
- wizualna informacja o pełnym związaniu szkicu,
- automatyczne lub półautomatyczne nakładanie więzów,
- możliwość dimensioningu „w locie”,
- szybki dostęp do komend szkicowych z drugiego poziomu menu radialnego. [A6] [A24] [A25]

### 5.2. Dlaczego szkic w Fusion wydaje się „lekki”

To nie wynika z magicznie prostszego solvera. Wygoda bierze się z kilku rzeczy naraz:

1. **Użytkownik widzi stan szkicu bez czytania panelu**
   - profil zamknięty ma wypełnienie,
   - geometria rzutowana ma własny status,
   - więzy i wymiary można włączać/wyłączać bez wychodzenia z canvas.

2. **Narzędzia referencyjne są nisko tarciowe**
   - szkic na ścianie od razu ma kontekst tej ściany,
   - `Project` działa szybko i jawnie,
   - można decydować, czy projekcja ma być powiązana, czy nie.

3. **Orientacja widoku nie wymaga walki**
   - `Look At` jest pod ręką,
   - `Slice` rozwiązuje problem szkicowania „we wnętrzu” bryły.

4. **UI jest blisko miejsca pracy**
   - nie trzeba nieustannie kursować wzrokiem i myszą między obiektem, drzewem i dalekim task panelem.

### 5.3. Co wprowadzić do FreeCAD

FreeCAD ma już dobre fundamenty:
- szkic jako główny nośnik intencji,
- aktywne więzy i wymiary,
- informację o stopniach swobody,
- auto constraints,
- nową sekcję parametrów narzędzi w Sketcher Dialog od wersji 1.0,
- sensowne mechanizmy solvera. [F3]

Problem nie leży więc w samym szkicowniku, tylko w ergonomii otaczającej szkic.

#### Rekomendacja dla moda FreeCAD

**Warstwa UX szkicu powinna zostać przebudowana na wzór Fusion, ale na bazie obecnego Sketchera:**

1. **Pływająca paleta szkicu w canvas**
   - `Look At`,
   - `Slice`,
   - siatka/snap,
   - profile,
   - więzy,
   - wymiary,
   - geometria zewnętrzna/projekcje,
   - 3D sketch.

2. **Auto-project krawędzi ściany przy szkicu na ścianie**
   - z wyborem: *linked* / *unlinked*,
   - z jasnym kodem wizualnym.

3. **DOF meter i stan szkicu w obszarze roboczym**
   - nie tylko w task panelu,
   - możliwość kliknięcia „pokaż nieswobodne / pokaż nadmiarowe”.

4. **Dimensioning on the go**
   - po narysowaniu linii/okręgu od razu mały input wymiaru obok kursora,
   - Enter zatwierdza i kontynuuje.

5. **Lepsza inferencja i sugestie**
   - wyróżnianie sugerowanego więzu przed kliknięciem,
   - przejrzyste badge’e nad geometrią.

6. **Szybkie wyjście i wejście**
   - `Finish Sketch` dostępne pod prawym przyciskiem i z małego mini-toolbaru w pobliżu szkicu,
   - nie tylko w task panelu.

### 5.4. Wniosek strategiczny

> **Jeżeli użytkownik ma w FreeCAD poczuć „Fusion-like sketching”, nie trzeba wymieniać solvera. Trzeba zrobić szkic bardziej lokalny, bardziej przewidywalny i bardziej wizualny.**

---

## 6. Konstrukcja pomocnicza (planes, axes, points) jako pełnoprawny element workflow

Fusion traktuje konstrukcyjne płaszczyzny, osie i punkty nie jako „narzędzia techniczne dla wtajemniczonych”, ale jako normalne wejścia do kolejnych operacji. Typowy przykład: płaszczyzna wzdłuż ścieżki, płaszczyzna pośrednia, offset plane – wszystko to służy do tworzenia szkiców, sweepów, split body, mirror itd. [A16]

### Dlaczego to jest wygodne

- użytkownik nie buduje geometrii zastępczej tylko po to, żeby wykonać jedną operację,
- konstrukcja pomocnicza jest lekka i naturalnie osadzona w historii,
- te obiekty są traktowane jak normalne referencje.

### Co powinien zrobić mod FreeCAD

FreeCAD ma już datum planes/axes/points, więc ponownie problem nie dotyczy mocy narzędzi, tylko ich „tarcia”.

#### Rekomendacja

- uprościć tworzenie datumów w jednym zunifikowanym panelu `Construct`,
- dać lepszy live preview w canvas,
- wspierać wejście „najpierw wybierz geometrię, potem uruchom komendę”,
- trzymać konstrukcję blisko aktywnego kontekstu (komponentu/body),
- pokazywać w mini panelu „co to za referencja i do czego może być użyta”.

---

## 7. Bryłowe feature’y tworzące geometrię

To jest serce modelowania precyzyjnego.

### 7.1. Extrude

`Extrude` w Fusion potrafi pracować na profilach szkicu i ścianach, wspiera m.in.:
- ekstruzję klasyczną,
- thin extrude,
- kierunek jednostronny / dwustronny / symetryczny,
- różne typy zakresu,
- taper,
- spójny wybór operacji rezultatu. [A12]

Dodatkowo, jeżeli w modelu jest tylko jeden widoczny profil, Fusion może go wskazać automatycznie. [A12]

#### Dlaczego jest to przyjemne

- jedno narzędzie pokrywa większość codziennych przypadków,
- logika operacji nie zmienia się między poleceniami,
- użytkownik ma jednocześnie:
  - bezpośredni manipulator,
  - dokładne pole liczbowe,
  - podgląd końcowego efektu.

#### Wniosek dla FreeCAD

To jest **jeden z najważniejszych punktów do skopiowania**.

Dziś FreeCAD ma świetne możliwości, ale z perspektywy początkującego zbyt często rozdziela mentalnie to, co w Fusion jest jednym poleceniem:
- Pad,
- Pocket,
- Part Extrude,
- Additive/Subtractive,
- osobne sposoby tworzenia nowego ciała.

##### Rekomendacja

Zbudować **jeden użytkowy front-end „Extrude”**, który wewnętrznie może dispatchować do obecnych obiektów FreeCAD, ale dla użytkownika wygląda tak:

- **Wejście**: profil / ściana,
- **Kierunek**: one side / two sides / symmetric,
- **Zakres**: distance / up to face / through all / up to next,
- **Rezultat**:
  - Add / Join,
  - Cut,
  - Intersect,
  - New Body,
  - New Component,
- **Preview**,
- **Manipulator**,
- **Dokładna wartość / wyrażenie**.

> Ważne: nie chodzi o kasowanie istniejących obiektów FreeCAD.  
> Chodzi o stworzenie nad nimi **jednej spójnej warstwy polecenia**.

### 7.2. Revolve, Sweep, Loft

Fusion stosuje bardzo podobny schemat do `Revolve`, `Sweep` i `Loft`. Najważniejsze jest nie to, że każde z tych narzędzi ma dużo opcji, ale to, że wszystkie są rozumiane podobnie:
- wybieram wejścia (profil, oś, ścieżkę, sekcje),
- widzę preview,
- ustawiam orientację / continuity / guide’y,
- wybieram **ten sam typ operacji rezultatu**. [A14] [A16]

`Loft` jest szczególnie ważny, bo pokazuje, jak Fusion skaluje z prostoty do zaawansowania:
- dla prostego przypadku wystarczą sekcje,
- dla trudniejszego dochodzą rails i centerline,
- ale użytkownik nie opuszcza tego samego modelu pracy.

#### Dlaczego to działa

Fusion nie próbuje „uprościć geometrii”. Uproszcza:
- kolejność decyzji,
- słownictwo,
- wygląd dialogów,
- przewidywalność wyniku.

#### Wniosek dla FreeCAD

Dla sweep/revolve/loft najważniejsze są:
1. **jednolity schemat dialogów**,
2. **spójna terminologia operacji**,
3. **zawsze aktywny preview**,
4. **ta sama logika edycji po fakcie**.

---

## 8. Press Pull – najważniejszy detal UX w całym Fusion

### 8.1. Co robi Press Pull

`Press Pull` jest narzędziem adaptacyjnym:
- wybór **profilu szkicu** uruchamia logikę extrude,
- wybór **krawędzi** uruchamia logikę fillet,
- wybór **ściany** uruchamia logikę offset face. [A5]

W efekcie użytkownik nie musi na początku idealnie nazwać intencji narzędziem. Najpierw wskazuje geometrię, potem system interpretuje sens działania.

### 8.2. Dlaczego to jest tak ważne

To narzędzie robi trzy rzeczy naraz:

1. **obniża koszt mentalny wejścia**  
   Początkujący nie musi pamiętać, czy chce „offset face” czy „press pull” czy „move face”. Po prostu „ciągnie” geometrię.

2. **ujawnia modelowanie jako manipulację obiektem**  
   Interakcja staje się bezpośrednia, nie formularzowa.

3. **ułatwia przejście od początkującego do zaawansowanego**  
   Ekspert nadal wie, że pod spodem finalnie powstaje konkretny feature, ale początkujący nie musi zaczynać od tej abstrakcji.

### 8.3. Jak przenieść to do FreeCAD

To powinno być jedno z pierwszych poleceń moda.

#### Proponowana implementacja

Utworzyć meta-komendę roboczo nazwaną `Quick Shape Edit` (jeżeli projekt nie chce kopiować nazwy `Press Pull`), która:

- dla **profilu szkicu** -> otwiera zunifikowany `Extrude`,
- dla **krawędzi bryły** -> otwiera `Fillet/Chamfer` w zależności od trybu lub modyfikatora,
- dla **ściany bryły** -> uruchamia `Offset Face`,
- dla **ściany planarne z predefiniowanym kierunkiem** -> opcjonalnie `Move Face`,
- zawsze pokazuje manipulator i value input.

#### Kluczowa zasada

W historii nie powinien powstawać „tajemniczy QuickShapeEditFeature”, tylko **kanoniczny typ feature’u**:
- Extrude,
- Fillet,
- Offset Face,
- Move Face.

To daje wygodę przy wejściu i klarowność przy późniejszej edycji.

### 8.4. To jest różnica między „mocnym CAD-em” a „przyjaznym mocnym CAD-em”

> **Dla użytkownika przyjemność nie bierze się z mniejszej liczby funkcji. Bierze się z mniejszej liczby decyzji, które musi podjąć przed pierwszym ruchem myszą.**

---

## 9. Hole, fillet, chamfer, shell, draft – modyfikatory codziennego użytku

### 9.1. Hole i Thread

Fusion posiada wyspecjalizowane narzędzia `Hole` i `Thread`, które prowadzą użytkownika przez gotowe wzorce otworów:
- simple,
- counterbore,
- countersink,
- ustawienia typu gwintu,
- długość, offset, modelowane vs kosmetyczne gwinty. [A15]

#### Dlaczego to jest wygodne

- użytkownik nie musi budować każdego otworu ze szkicu + pocket + dodatkowych zabiegów,
- semantyka „to jest otwór technologiczny” jest jawna,
- łatwiej to potem patternować, dokumentować i produkcyjnie interpretować.

#### Co dodać do FreeCAD

- jeden zunifikowany `Hole Tool` z semantyką technologiczną,
- osobna warstwa „cosmetic vs modeled thread”,
- możliwość edycji otworu jako obiektu, a nie tylko jako rezultatu operacji odejmowania.

### 9.2. Fillet i Chamfer

W Fusion fillet/chamfer są prowadzone bardziej jak lokalne operacje na krawędziach niż „specjalne wydarzenia”. `Press Pull` dodatkowo skraca do nich wejście. [A5]

#### Co jest istotne dla FreeCAD

- szybkie wejście z preselection,
- podgląd na żywo,
- możliwość zmiany zestawu zaznaczenia w trakcie,
- tangent chain jako świadomy przełącznik,
- spójny edytor po fakcie.

### 9.3. Shell i Draft

To narzędzia mniej „medialne”, ale bardzo ważne dla workflow przemysłowego. Fusion traktuje je jako naturalne, parametryczne feature’y, a nie odrębny egzotyczny zestaw poleceń.

#### Rekomendacja dla FreeCAD

- włączyć je do tej samej rodziny `Modify`,
- dać wspólny wygląd dialogu,
- dodać dobry preview,
- zadbać o czytelny komunikat błędów, gdy operacja nie może zostać rozwiązana.

---

## 10. Pattern, Mirror, Boolean, Split, Boundary Fill

### 10.1. Pattern i Mirror

Fusion pozwala patternować i mirrorować różne poziomy obiektów:
- faces,
- bodies,
- features,
- components. [A13] [A14]

To jest niezwykle ważne, bo użytkownik nie musi zastanawiać się, czy „to jest pattern szkicu, bryły czy feature’u” jako osobna filozofia programu. Najpierw wskazuje **typ obiektu**, a potem wykonuje powielenie.

#### Dlaczego to jest przyjemne

- ten sam wzorzec narzędzia działa w wielu skalach,
- użytkownik nie uczy się kilku różnych rodzin „powielania”,
- łatwiej myśleć w kategoriach: „powiel ten rezultat na osi / po ścieżce”.

#### Wniosek dla FreeCAD

FreeCAD powinien dostać **jeden zunifikowany interfejs Pattern/Mirror**, gdzie użytkownik wybiera:
- typ wejścia,
- kierunek/oś/ścieżkę,
- liczbę,
- rozstaw / zasięg,
- ewentualne suppress instancji.

### 10.2. Boundary Fill i Boolean

`Boundary Fill` w Fusion jest genialnym przykładem zaawansowania bez chaosu:
- wybierasz bryły/płaszczyzny/powierzchnie,
- system wykrywa zamknięte komórki,
- wskazujesz komórkę do zachowania albo odjęcia,
- operacja kończy się znowu tym samym słownikiem rezultatu. [A16]

To bardzo upraszcza trudniejsze przypadki przestrzenne.

#### Wniosek dla FreeCAD

Tu nie chodzi tylko o samo narzędzie. Chodzi o to, by **operacje boolowskie i objętościowe były przedstawione użytkownikowi jako jeden spójny świat**:
- wybór narzędzi,
- wybór wyniku,
- preview,
- jawni uczestnicy operacji.

### 10.3. Uwaga praktyczna dla FreeCAD

Fusion w części operacji `Cut` domyślnie używa widoczności ciał do określenia uczestników, a potem potrafi zapamiętać listę uczestników albo przeliczać ją automatycznie. [A12] [A16]

To jest wygodne, ale może też zaskakiwać.

#### Lepszy pomysł dla moda FreeCAD

- dać **smart auto-select** jako domyślne zachowanie,
- ale zawsze pokazywać **jawny panel uczestników**,
- przy edycji feature’u pozwolić przełączyć:
  - `Auto`,
  - `Zamrożona lista uczestników`.

To daje wygodę Fusion bez utraty czytelności.

---

## 11. Direct Modeling – jak Fusion rozwiązuje „lokalną chirurgię” geometrii

### 11.1. Dwa tryby modelowania

Fusion rozróżnia:
- **Parametric Modeling Mode** – z timeline,
- **Direct Modeling Mode** – bez parametrycznego śledzenia feature’ów. [A1] [A2] [A3]

Przejście do direct mode usuwa timeline i przenosi rozpoznawalne feature’y do Browsera; nowe operacje nie są śledzone parametrycznie. [A2]

Importy z innych systemów CAD i formaty neutralne często trafiają właśnie do takiego modelu bez historii; można później częściowo odzyskać semantykę przez `Find Features`. [A23]

### 11.2. Dlaczego to jest przyjemne

W praktyce użytkownik ma dwie ścieżki:
- gdy projektuje od zera i chce kontrolować intencję -> pracuje historycznie,
- gdy dostaje cudzy STEP albo potrzebuje szybkiego lokalnego „naciągnięcia” ściany -> działa bezpośrednio.

To bardzo ważne, bo realny CAD rzadko jest czystym, akademickim modelem parametrycznym.

### 11.3. Direct edit w praktyce Fusion

Do kluczowych operacji należą:
- `Move/Copy`,
- `Offset Face`,
- `Delete Face` / heal,
- `Replace Face`,
- `Edit Face` z użyciem logiki T-Spline dla lokalnej zmiany kształtu. [A18]

Są to narzędzia „naprawcze”, „adaptacyjne”, „importowe” i koncepcyjne zarazem.

### 11.4. Czego NIE kopiować 1:1 do FreeCAD

Najgorsza rzecz do bezrefleksyjnego skopiowania z Fusion to **destrukcyjne przełączanie trybu**, które usuwa historię parametryczną. [A2]

Dla moda FreeCAD zalecam znacznie lepszy wzorzec:

#### Zamiast „tryb bez historii”:
- wprowadzić **feature’y direct edit jako normalne węzły historii**,
- albo wydzielone „base feature islands”,
- albo hybrydowe kontenery lokalnej edycji bezpośredniej.

#### Przykład
Zamiast:
- „przełącz projekt na direct mode, stracisz timeline”,

lepiej:
- „dodaj feature `Offset Face` / `Move Face` / `Delete Heal Face` do historii aktywnego body”.

Wtedy:
- zachowujesz edytowalność,
- zachowujesz rollback,
- nie zrywasz całego modelu mentalnego użytkownika.

### 11.5. Minimalny zestaw direct edit dla FreeCAD

P0:
- Move Face,
- Offset Face,
- Delete + Heal Face,
- Replace Face,
- Move/Copy Body.

P1:
- lokalna edycja ściany planarne/obrotowe przez manipulator,
- inteligentny drag handles na wybranych ścianach,
- częściowa rekonstrukcja feature’ów z importu.

P2:
- lokalna edycja typu form/SubD na zaznaczonej ścianie (odpowiednik idei `Edit Face`).

---

## 12. T-Spline / Form / Surface – dlaczego Fusion jest jednocześnie prosty i zaawansowany

### 12.1. Różne typy ciał

Fusion jawnie rozróżnia typy ciał:
- solid,
- surface,
- sheet metal,
- T-Spline,
- mesh. [A20]

Dokumentacja wprost zaznacza, że różne typy ciał używają różnej matematyki i nie wszystkie mogą ze sobą bezpośrednio wchodzić w operacje; często trzeba je konwertować do tego samego typu. [A20]

### 12.2. Jak działa Form/T-Spline

Fusion pozwala pracować na T-Spline body w środowisku Form, a potem konwertować je do reprezentacji boundary representation / solid do dalszego modelowania klasycznymi narzędziami. [A17]

Jest też lokalna edycja ściany przy użyciu logiki T-Spline w direct mode. [A18]

### 12.3. Dlaczego to zwiększa przyjemność

Bo użytkownik nie musi wybierać na starcie „albo organiczne, albo inżynierskie”.  
Może:
- zbudować formę bardziej rzeźbiarsko,
- zamienić ją na bryłę,
- dokończyć otwory, shelle, splitty, booleany i inne operacje techniczne.

To daje poczucie dużej mocy bez konieczności zmiany programu.

### 12.4. Co zrobić w FreeCAD

FreeCAD nie musi kopiować T-Spline dosłownie. Nie powinien też kopiować nazewnictwa/trademarków 1:1.  
Powinien natomiast uzyskać ten sam efekt użytkowy:

#### Rekomendacja

1. **Wspólny shell „Design” z zakładkami**
   - Sketch,
   - Solid,
   - Surface,
   - Form/SubD,
   - Inspect,
   - Assembly.

2. **Konwersja między reprezentacjami**
   - surface -> solid,
   - subD/form -> surface/solid,
   - mesh -> solid/surface tam, gdzie to ma sens.

3. **Lokalny workflow hybrydowy**
   - użytkownik nie ma czuć, że wychodzi do innego świata.

4. **Nie kopiować technologii, tylko zdolność**
   - dla open source lepiej myśleć o *subdivision/form modeling* niż o dosłownym powielaniu T-Splines.

---

## 13. Browser, Bodies, Components – jeden z najbardziej niedocenianych powodów wygody Fusion

### 13.1. Czym jest body w Fusion

W Fusion body to „single container for a contiguous 3D shape” – pojedyncza, spójna bryła/kształt 3D. Każde body należy do komponentu; komponent może mieć wiele body, ale ruch i połączenia są domeną komponentów, nie body. [A20]

### 13.2. Aktywny komponent

Gdy użytkownik aktywuje komponent:
- timeline filtruje się do feature’ów tego komponentu,
- rodzic i rodzeństwo stają się półprzezroczyste,
- nowe szkice, body i geometria pomocnicza trafiają do właściwego miejsca. [A1] [A19]

### 13.3. Dlaczego to jest wygodne

To rozwiązuje kilka problemów jednocześnie:
- zmniejsza ryzyko modelowania „nie tam gdzie trzeba”,
- porządkuje historię,
- daje jasne odróżnienie:
  - **body = region geometrii**,
  - **component = jednostka strukturalna / montażowa**.

### 13.4. Kluczowa różnica względem FreeCAD

W FreeCAD użytkownik bardzo szybko trafia na rozróżnienia:
- Document,
- Part,
- Body,
- Link,
- Assembly,
- obiekty z różnych workbenchów.

To jest potężne, ale dla wielu osób nieintuicyjne.

Do tego **PartDesign Body jest sekwencją feature’ów tworzących pojedynczą bryłę**. [F2]

To ma zalety, ale oznacza też, że workflow wielobryłowy jest mniej naturalny niż w Fusion.

### 13.5. Co zrobić w modzie FreeCAD

To jest jedna z najbardziej strategicznych decyzji projektowych.

#### Rekomendacja wysokiego poziomu

W warstwie UX wprowadzić prostszy model:

- **Component** – użytkowy kontener części/zespołu,
- **Bodies** – jedna lub wiele brył w komponencie,
- **Features / Sketches / Construction** – elementy wewnątrz aktywnego body lub komponentu.

#### A co z obecnymi obiektami FreeCAD?

Nie trzeba ich usuwać. Można:
- zachować obecne klasy jako backend,
- ale na froncie pokazać uproszczoną semantykę.

#### Opcja praktyczna dla moda

- utworzyć **zunifikowany „Model Browser”**,
- w trybie podstawowym ukryć część technicznej złożoności App::Part/Body/Link,
- w trybie eksperckim odsłonić pełne drzewo FreeCAD.

### 13.6. Dodatkowa rekomendacja

Aktywny kontekst powinien:
- być wyraźnie zaznaczony wizualnie,
- filtrować listę feature’ów,
- wygaszać geometrię spoza kontekstu,
- decydować, gdzie trafiają nowe obiekty.

> **FreeCAD ma już zalążek tej idei w aktywnym Body. Trzeba tę ideę uogólnić na cały workflow.**

---

## 14. Timeline – dlaczego historia Fusion jest tak czytelna

### 14.1. Co robi Timeline

Timeline w Fusion zapisuje feature’y tworzące model i ich parametryczną historię. Pozwala:
- przechodzić w przód/wstecz,
- suppressować feature’y,
- przenosić je w czasie,
- filtrować po aktywnym komponencie. [A1]

### 14.2. Dlaczego to działa lepiej niż samo drzewo

Bo timeline odpowiada na inne pytanie niż tree/browser.

- **Browser** odpowiada: „z czego składa się model?”  
- **Timeline** odpowiada: „w jakiej kolejności i przez jakie decyzje powstał model?”

To jest gigantyczna poprawa czytelności.

### 14.3. Problem FreeCAD

FreeCAD jest bardziej jawny obiektowo, ale czas powstawania feature’ów i relacje przebudowy są dla użytkownika mniej namacalne.  
Drzewo bardzo często miesza:
- strukturę,
- zależności,
- wynik,
- historię.

### 14.4. Rekomendacja dla moda FreeCAD

Nie trzeba budować stuprocentowej kopii timeline Fusion. Wystarczy bardzo dużo zyskać dzięki:

#### `History Strip` dla aktywnego kontekstu

Minimalna wersja:
- poziomy pasek feature’ów aktywnego Body/Component,
- możliwość:
  - rollback,
  - edit,
  - suppress,
  - reorder tam, gdzie zależności na to pozwalają,
- badge’e błędów i ostrzeżeń.

#### Dlaczego to wystarczy

Bo FreeCAD już ma obiekty i zależności. Brakuje głównie:
- **czytelnej reprezentacji liniowego toku projektowania**,  
- szybkiej obsługi tej reprezentacji przez użytkownika.

---

## 15. Parametry i wyrażenia – precyzja bez straszenia

### 15.1. Jak to robi Fusion

Fusion ma `Change Parameters`, ale kluczowe jest coś więcej:
- parametry użytkownika,
- parametry modelu,
- wyrażenia,
- jednostki w polach,
- możliwość tworzenia parametrów **w locie** w aktywnym polu, np. `Width=50`, `Length=Width*2`. [A21] [A22]

### 15.2. Dlaczego to jest tak ważne

Bo w Fusion nie ma ostrego podziału:
- „teraz modelujesz intuicyjnie”,
- „a teraz wchodzisz do mrocznej, eksperckiej tabeli parametrów”.

Parametry są obecne wszędzie, ale nie są narzucane.

### 15.3. Co z FreeCAD

FreeCAD ma potężny silnik wyrażeń i jest pod tym względem bardzo mocny. Problemem bywa raczej **przystępność i wszechobecność interfejsową**, a nie sama możliwość.

### 15.4. Rekomendacja

Każde pole liczbowe w nowej warstwie modelowania FreeCAD powinno:
- przyjmować liczbę,
- liczbę z jednostką,
- wyrażenie,
- istniejący parametr,
- definicję nowego parametru (`Name=Value` lub `Name=Expression`).

Do tego:
- mały przełącznik `fx`,
- autouzupełnianie parametrów,
- panel ulubionych parametrów,
- możliwość chwilowego wyłączenia automatycznej przebudowy przy masowej edycji.

> **To jest jeden z tych elementów, które sprawiają, że system wygląda jednocześnie prosto i „profesjonalnie”.**

---

## 16. Selekcja – niewidzialny fundament wygody

### 16.1. Co potrafi Fusion

Fusion ma:
- filtry selekcji po typach obiektów,
- priorytety selekcji (component/body/face/edge),
- `Select Through`,
- różne tryby selekcji obszarowej,
- wybór obiektów ukrytych za innymi poprzez listę `Depth/Parents`. [A9] [A10] [A11]

### 16.2. Dlaczego to robi tak dużą różnicę

W trudnym CAD-zie użytkownik bardzo często nie cierpi z powodu geometrii. Cierpi z powodu:
- złego wyboru ściany,
- niemożności kliknięcia obiektu za innym,
- niejasności, czy wybiera feature, face czy całe body,
- przypadkowych preselection.

Fusion zdejmuje sporą część tego bólu.

### 16.3. Co ma już FreeCAD

FreeCAD 1.0 przeszedł istotne usprawnienia UI, w tym m.in. selection item filters i on-model task panel. [F1]  
To bardzo ważny sygnał: kierunek jest właściwy.

### 16.4. Co należy zrobić dalej

#### Rekomendacja P0

1. **Globalny pasek/sekcja Selection**
   - priority: component / body / face / edge / vertex,
   - select through,
   - paint / window / crossing / freeform.

2. **Depth picker**
   - po przytrzymaniu kliknięcia lista obiektów pod kursorem,
   - rozróżnienie: subelement / body / feature / komponent.

3. **Preselection jako pełnoprawny stan**
   - lepszy highlight,
   - mini opis obiektu pod kursorem,
   - informacja, do jakich operacji nadaje się zaznaczenie.

4. **Selection memory**
   - możliwość modyfikacji zestawu zaznaczenia bez resetu komendy,
   - spójne zachowanie między wszystkimi poleceniami.

---

## 17. Nawigacja – mechanika działania myszy, klawiatury i kamery

### 17.1. Model nawigacji w Fusion: mouse-first, camera-centric

Oficjalna referencja skrótów Fusion rozdziela interakcję na trzy warstwy: **Tool**, **System** oraz **Canvas Selection**. W warstwie Canvas Selection nawigacja 3D jest obsługiwana niemal wyłącznie przez mysz:
- `Middle Mouse Button` = pan,
- rolka środkowego przycisku = zoom,
- `Shift + Middle Mouse Button` = orbit,
- `Shift + Click + Middle Mouse Button` = orbit around point. [A26]

To nie jest przypadkowy zestaw skrótów. To architektura interakcji: kamera nie wymaga osobnego narzędzia ani osobnego „trybu widoku”. Z perspektywy projektowej oznacza to, że użytkownik może przez większość czasu pozostać w aktywnej komendzie modelowania, a mimo to stale korygować widok bez zmiany kontekstu pracy.  

W efekcie Fusion nie buduje osobnej „fazy nawigacji”, tylko traktuje ruch kamery jako równoległą warstwę dostępną cały czas.

### 17.2. Domyślny preset Fusion – dokładne zachowanie myszy i klawiatury

Poniżej znajduje się mechanika zachowania domyślnego presetu `Fusion` według oficjalnej dokumentacji:

| Funkcja | Wejście | Co robi system | Znaczenie dla workflow |
|---|---|---|---|
| Pan | Przytrzymanie środkowego przycisku myszy i przeciągnięcie | Przesuwa kamerę równolegle do płaszczyzny widoku | Umożliwia lokalne „dostawienie” modelu bez zmiany punktu obrotu [A26] |
| Zoom | Ruch rolką | Zmienia powiększenie | Nie wymaga przełączania narzędzia [A26] |
| Orbit | `Shift + środkowy przycisk myszy` | Obraca kamerę wokół bieżącego pivotu | Standardowy obrót 3D w czasie modelowania [A26] |
| Orbit around point | `Shift + klik środkowym przyciskiem myszy` | Przejmuje punkt wskazany w canvas jako punkt odniesienia orbity | Kluczowe przy pracy na lokalnym detalu, we wnętrzu modelu lub na małej ścianie [A26] |
| Fit view | `F6` | Dopasowuje widok do aktualnego okna | Najszybszy mechanizm odzyskania modelu po „zgubieniu” kamery [A27] |
| Show/Hide ViewCube | `Ctrl + Alt + V` | Włącza/wyłącza widget orientacji | Pozwala odzyskać czystszy canvas lub szybciej wrócić do orientacji [A26] |
| Show/Hide Navigation Bar | `Ctrl + Alt + N` | Włącza/wyłącza pasek nawigacji | Redukuje lub zwiększa liczbę widocznych pomocy nawigacyjnych [A26] |
| Toolbox | `S` | Otwiera lokalny toolbox / command search dla modelowania | Pozostawia rękę na klawiaturze jako akceleratorze komend, nie kamery [A26] |
| Marking Menu | Prawy przycisk myszy | Otwiera radialne menu kontekstowe | Integruje komendy, nawigację, izolację i skróty w pobliżu kursora [A6] |

Ważne: oficjalna lista skrótów Fusion nie opisuje rozbudowanego „pełnego sterowania kamerą z klawiatury” w stylu aplikacji DCC czy części innych CAD-ów. Dokumentacja pokazuje raczej model, w którym **mysz steruje kamerą, a klawiatura steruje poleceniami**. [A26]

### 17.3. Presety zgodności – jak Fusion minimalizuje koszt migracji użytkownika

W preferencjach Fusion można przełączyć zachowanie PZO (`Pan, Zoom, Orbit shortcuts`) tak, aby odpowiadało innym programom. Dokumentacja wymienia presety: **Fusion**, **Alias**, **Inventor**, **SolidWorks**, **Tinkercad** i **PowerMill**. Dla każdego presetu Autodesk podaje konkretną mapę myszy i modyfikatorów. [A27]

To rozwiązanie ma bardzo duże znaczenie projektowe:
- użytkownik nie musi od razu porzucać dotychczasowej pamięci mięśniowej,
- migracja z innych CAD-ów nie wymaga jednoczesnej nauki geometrii i nowej obsługi kamery,
- koszt wejścia jest obniżany bez rozbijania spójności wnętrza programu.

Dla projektu moda FreeCAD kluczowy wniosek brzmi: **presety zgodności nie są dodatkiem kosmetycznym, tylko elementem strategii adopcyjnej**.

### 17.4. Klawiatura w Fusion: warstwa wywoływania intencji, nie substytut myszy

Oficjalna referencja skrótów Fusion pokazuje, że klawiatura jest wykorzystywana głównie do uruchamiania poleceń, przełączania elementów UI i wyboru trybów zaznaczania. Najważniejsze skróty z perspektywy modelowania to m.in.:
- `E` = Extrude,
- `H` = Hole,
- `Q` = Press Pull,
- `F` = Fillet,
- `M` = Move,
- `L` = Line,
- `R` = 2-point Rectangle,
- `C` = Center Diameter Circle,
- `T` = Trim,
- `O` = Offset,
- `I` = Measure,
- `P` = Project,
- `X` = Normal / Construction,
- `D` = Sketch Dimension,
- `1`, `2`, `3` = odpowiednio Window / Freeform / Paint Selection,
- `Del` = Delete,
- `Ctrl + [` oraz `Ctrl + ]` = przełączanie workspace’ów. [A26]

Wniosek systemowy jest istotny: Fusion implementuje **dwuręczny model pracy**:
- prawa ręka steruje kamerą i selekcją,
- lewa ręka wywołuje narzędzia, tryby wyboru i przełączniki interfejsu.

To sprawia, że kamera nie „zjada” budżetu klawiaturowego. Klawisze pozostają wolne dla operacji geometrycznych i organizacyjnych. Z punktu widzenia ergonomii jest to jeden z powodów, dla których Fusion jest odbierany jako szybki nawet przez użytkowników, którzy pracują bardzo precyzyjnie i bardzo często przełączają narzędzia.

### 17.5. Pivot, orientacja szkicu i odzyskiwanie kontroli nad kamerą

Fusion nie opiera wygody nawigacji wyłącznie na samym mapowaniu myszy. Równie ważne są reguły odzyskiwania orientacji i zarządzania punktem obrotu.

#### 17.5.1. Orbit around point
`Shift + klik środkowym przyciskiem myszy` pozwala przejąć punkt z canvas jako punkt odniesienia orbity. [A26]

To rozwiązuje bardzo konkretny problem: gdy użytkownik pracuje na małym detalu, we wnęce, na cienkiej ściance albo na geometrii daleko od środka obiektu, obrót wokół środka całego modelu staje się nieefektywny. Fusion daje mechanizm lokalnej zmiany pivotu bez opuszczania bieżącej komendy.

#### 17.5.2. Look At
W Sketch Palette dostępna jest funkcja `Look At`, która ustawia kamerę prostopadle do aktywnej płaszczyzny szkicu. [A7]

To oznacza, że orientacja szkicu nie jest pozostawiona domysłom ani ręcznej manipulacji ViewCube’em. System ma jawny, natychmiastowy mechanizm przejścia do „widoku roboczego” szkicu.

#### 17.5.3. Slice
Sketch Palette udostępnia `Slice`, które tymczasowo przecina bryły na płaszczyźnie aktywnego szkicu. [A7]

To nie jest tylko narzędzie wizualne. Z punktu widzenia nawigacji usuwa problem „rysowania przez bryłę” i redukuje potrzebę ciągłego orbitowania tylko po to, aby zobaczyć aktywny przekrój.

#### 17.5.4. Fit
Preferencje Fusion dokumentują skrót `F6` jako szybkie dopasowanie widoku do okna. [A27]

W praktyce jest to podstawowy „panic button” odzyskujący kontrolę nad kamerą. Dobrze zaprojektowany system modelowania zawsze potrzebuje takiego mechanizmu, bo nawet dobra orbita nie eliminuje przypadków zgubienia modelu.

### 17.6. Warstwa pomocnicza wokół kamery: Marking Menu, ViewCube, pasek nawigacji, animacje

Fusion rozprowadza funkcje nawigacyjne między kilka warstw, ale robi to w sposób spójny:

1. **Bezpośrednia warstwa sterowania kamerą**  
   Pan/zoom/orbit/orbit around point są stale pod ręką na myszy. [A26]

2. **Warstwa lokalna pod kursorem**  
   Marking Menu zawiera nie tylko komendy modelujące, ale też kontrolki nawigacyjne (`Pan`, `Zoom`, `Orbit`), `Isolate/Unisolate`, listę workspace’ów oraz zapisane skróty. [A6]

3. **Warstwa kontekstowa szkicu**  
   Sketch Palette zawiera `Look At`, `Slice`, grid, snap i przełączniki widoczności profili, punktów, wymiarów, więzów oraz projekcji. [A7]

4. **Warstwa trwałych widgetów**  
   ViewCube i Navigation Bar można szybko pokazać lub ukryć skrótami klawiaturowymi. [A26]

5. **Warstwa percepcji płynności**  
   W preferencjach graficznych Fusion są opcje `Performance`, `Quality`, `Custom`, dynamiczne obniżanie jakości w czasie nawigacji, minimalny FPS podczas nawigacji oraz `Animate view transitions`. [A27]

Ta piąta warstwa jest często pomijana w analizach, a ma ogromne znaczenie. Użytkownik odbiera nawigację jako „dobrą” nie tylko wtedy, gdy skróty są sensowne, ale także wtedy, gdy:
- kamera reaguje płynnie,
- przejścia widoku nie gubią orientacji,
- obciążenie graficzne nie powoduje nagłych spadków klatek podczas orbity.

### 17.7. FreeCAD – aktualny model nawigacji i wyboru

FreeCAD ma dziś bardzo rozbudowany i formalnie bardziej pluralistyczny system nawigacji niż Fusion. Dokumentacja opisuje wiele stylów globalnych, wybieranych ze status bara, menu kontekstowego 3D view albo z preferencji. Wybrany styl obowiązuje we wszystkich workbenchach. [F4]

#### 17.7.1. Domyślny styl CAD Navigation w FreeCAD
Według dokumentacji FreeCAD domyślny styl `CAD Navigation` działa następująco:
- lewy przycisk myszy = wybór,
- rolka = zoom,
- klik środkowym przyciskiem myszy = ponowne wyśrodkowanie widoku na wskazanym miejscu,
- środkowy przycisk + ruch = pan,
- środkowy przycisk, a następnie lewy przycisk + ruch = obrót,
- alternatywnie środkowy przycisk, a następnie prawy przycisk + ruch = obrót,
- tryby prawoprzyciskowe z modyfikatorami `Ctrl` / `Shift` uruchamiają odpowiednio zoom / rotate / pan. [F4]

To oznacza, że FreeCAD już dziś posiada bardzo dużą funkcjonalność, ale jego domyślna semantyka jest inna niż w Fusion. W szczególności obrót wymaga bardziej złożonych kombinacji przycisków niż `Shift + MMB`.

#### 17.7.2. Pozostałe style nawigacji w FreeCAD
Dokumentacja wymienia również style: **Blender**, **Gesture**, **Maya-Gesture**, **OpenCascade**, **OpenInventor**, **OpenSCAD**, **Revit**, **Siemens NX**, **SolidWorks**, **TinkerCAD** oraz **Touchpad**. [F4]

To ma dwa skutki:
- FreeCAD jest bardzo elastyczny jako platforma,
- ale użytkownik częściej musi podjąć decyzję konfiguracyjną albo dopasować się do stylu, który nie odpowiada dokładnie jego pamięci mięśniowej.

#### 17.7.3. Klawiatura w FreeCAD
Dla wszystkich stylów dokumentacja FreeCAD przewiduje wspólne opcje klawiaturowe: zoom klawiszami `Ctrl +` / `Ctrl -` lub `PgUp` / `PgDn`, pan klawiszami strzałek oraz dodatkowe skróty do obrotu widoku o 90°. [F4]

Oprócz tego FreeCAD ma skróty do orientacji i widoczności, np.:
- `0` = widok izometryczny, [F7]
- `V` potem `S` = dopasowanie do zaznaczenia, [F8]
- `Ctrl++` = zoom in, [F9]
- `Space` = przełączanie widoczności zaznaczonych obiektów. [F10]

To pokazuje, że w FreeCAD klawiatura ma większy udział w bezpośrednim sterowaniu widokiem niż w Fusion.

#### 17.7.4. Navigation Cube i logika wyboru w FreeCAD
FreeCAD ma `Navigation Cube`, widoczny domyślnie w prawym górnym rogu. Cube posiada 26 klikalnych orientacji, strzałki kierunkowe, przycisk odwrócenia widoku oraz mini-menu z opcjami: orthographic, perspective, isometric, fit all, fit selection i align to selection. [F6]

Z kolei dokumentacja `Selection methods` opisuje charakterystyczną logikę narastającej selekcji:
- pierwszy klik wybiera podobiekt (wierzchołek / krawędź / ścianę),
- drugi klik wybiera cały obiekt,
- trzeci klik rozszerza wybór do kontenera (`Body`, `Std Part` itd.),
- kolejne kliknięcia idą dalej po łańcuchu kontenerów. [F5]

Jest to mocny mechanizm strukturalny, ale inny niż logika Fusion, która bardziej opiera się na filtrach, priorytetach i kontekstowych trybach selekcji. [A9] [A10] [F5]

### 17.8. Różnica architektoniczna: Fusion kontra FreeCAD

Na poziomie samych możliwości oba programy potrafią dużo. Różnica dotyczy głównie kompozycji tych możliwości.

#### Fusion
- jeden wyraźny, niski-ambiguitetowy preset domyślny,
- kamera sterowana głównie myszą,
- klawiatura zarezerwowana głównie dla poleceń,
- łatwy reset pivotu (`orbit around point`),
- lokalne odzyskiwanie orientacji przez `Look At`, `Slice`, `Marking Menu`, `Fit`,
- presety kompatybilności dla użytkowników innych CAD-ów. [A6] [A7] [A26] [A27]

#### FreeCAD
- wiele stylów globalnych,
- większa zmienność semantyki między stylami,
- szerszy udział klawiatury w sterowaniu widokiem,
- bardzo rozbudowany Navigation Cube,
- silna logika selekcji hierarchicznej i kontenerowej,
- brak natywnego presetu `Fusion` opisanego w dokumentacji. [F4] [F5] [F6] [F7] [F8] [F9]

Wniosek analityczny jest prosty: **FreeCAD nie ma słabego systemu nawigacji; ma system bardziej zróżnicowany i mniej znormalizowany pod konkretny model mentalny znany z Fusion**.

### 17.9. Co dokładnie powinno wejść do moda FreeCAD

Jeżeli celem jest odtworzenie sposobu pracy znanego z Fusion, to mod powinien zdefiniować nawigację nie jako „zbiór skrótów”, lecz jako spójny kontrakt interakcyjny.

#### Wymagania funkcjonalne dla warstwy nawigacji

1. **Dokładny preset `Fusion`**  
   - rolka = zoom,  
   - `MMB drag` = pan,  
   - `Shift + MMB drag` = orbit,  
   - `Shift + click + MMB` = reset pivotu / orbit around point.  
   Zachowanie powinno być mięśniowo zgodne z Fusion. [A26] [A27]

2. **Jawne reguły pivotu**  
   Mod powinien mieć określoną kolejność źródeł punktu obrotu, np.:
   - punkt wskazany explicite,
   - środek zaznaczenia,
   - środek aktywnej płaszczyzny szkicu,
   - środek extents modelu.

3. **Lokalne narzędzia odzyskiwania widoku**  
   - `Fit`,
   - `Look At active sketch plane`,
   - `Align to selection`,
   - `Slice at sketch plane`,
   - `Isolate / Unisolate`,
   - szybki powrót do widoku roboczego szkicu. [A6] [A7] [F6]

4. **Warstwa pod kursorem**  
   Prawy przycisk powinien otwierać radialne menu zawierające zarówno komendy modelowania, jak i funkcje widoku. To powinno ograniczyć przechodzenie do odległych paneli. [A6]

5. **Warstwa trwałych widgetów**  
   Navigation Cube w FreeCAD już istnieje. W modzie należy go potraktować jako element centralny, a nie poboczny. Powinien współgrać z `Fit`, `Fit Selection`, `Align to Selection`, ortho/perspective oraz aktywnym szkicem. [F6]

6. **Warstwa wydajnościowa**  
   Warto dodać ustawienia podobne do Fusion:
   - animacje przejść widoku,
   - minimalny FPS podczas nawigacji,
   - opcjonalne obniżanie jakości efektów podczas orbity. [A27]

7. **Rozdział roli myszy i klawiatury**  
   Aby uzyskać workflow bliższy Fusion, kamera powinna pozostać przede wszystkim na myszy, a klawiatura powinna zostać uwolniona dla poleceń, wyszukiwarki komend, selekcji i przełączników UI. [A26]

8. **Brak destrukcji obecnych stylów**  
   Obecne style FreeCAD są wartością samą w sobie. Mod nie powinien ich usuwać. Powinien dodać jeden nowy, bardzo precyzyjnie zdefiniowany profil, a nie zastąpić wszystkie istniejące zachowania. [F4]

### 17.10. Dlaczego to jest krytyczne dla projektu

W praktyce użytkownik bardzo szybko wyrabia sobie opinię o „trudności” systemu CAD na podstawie dwóch rzeczy:
- czy potrafi bez walki obrócić model i wrócić do właściwej orientacji,
- czy potrafi bez frustracji wskazać to, co chciał zaznaczyć.

Z tego powodu nawigacja i selekcja nie są warstwą pomocniczą, tylko częścią rdzenia doświadczenia modelowania. Dla projektu upodabniającego FreeCAD do Fusion sekcja nawigacji powinna być traktowana równie poważnie jak szkic, ekstrudowanie czy historię.

## 18. Marking Menu i toolbox `S` – przyspieszacze eksperta, które nie szkodzą początkującym

### 18.1. Marking Menu

W Fusion prawy przycisk otwiera radialne `Marking Menu` z najczęstszymi komendami, dostosowane do workspace’u i kontekstu. W Design są tam m.in. Repeat last command, Press Pull, Hole, Sketch, Move/Copy, Undo/Redo, Delete. Jest też drugi poziom komend szkicowych oraz gesty pozwalające uruchamiać je niemal bez patrzenia. [A6]

### 18.2. Toolbox `S`

Klawisz `S` otwiera toolbox / wyszukiwarkę narzędzi aktualnego workspace’u. [A26]

### 18.3. Dlaczego to działa

- początkujący nadal mogą korzystać z toolbaru,
- średniozaawansowany szybko odkrywa wygodniejszą ścieżkę,
- ekspert skraca dystans kursora i czas mentalny.

To jest klasyczny przykład **progressive disclosure**:
- niczego nie zabierasz początkującym,
- a eksperci dostają turbo.

### 18.4. Co zrobić w FreeCAD

#### P0
- `S` = command palette / search,
- prawy przycisk = radialne menu kontekstowe,
- ostatnia komenda pod jednym gestem,
- `Finish Sketch`, `Extrude`, `Hole`, `Fillet`, `Measure`, `Isolate`, `Look At`, `Fit` w pierwszej warstwie.

#### P1
- osobne układy radialnego menu dla:
  - szkicu,
  - brył,
  - powierzchni,
  - assembly,
  - inspect.

---

## 19. Dlaczego Fusion wydaje się uproszczony, a jednocześnie zaawansowany

To jest najważniejsza część całej analizy.

### 19.1. Fusion upraszcza **wejście**, a nie **możliwości**

Początkujący widzi mało:
- szkic,
- extrude,
- fillet,
- hole,
- pattern,
- mirror.

Ekspert widzi dużo więcej:
- continuity,
- guide rails,
- centerlines,
- body types,
- parametry,
- direct edit,
- form,
- surface,
- imported feature recognition.

To jest esencja **progressive disclosure**.

### 19.2. Mały zestaw stałych wzorców interakcji

Prawie wszystko w Fusion można opisać wzorem:

1. wybierz geometrię,
2. zobacz preview,
3. przeciągnij manipulator albo wpisz wartość,
4. wybierz typ rezultatu,
5. zatwierdź,
6. wróć później przez timeline/browser i edytuj.

To jest silniejszy powód wygody niż jakakolwiek pojedyncza komenda.

### 19.3. Struktura i historia są czytelne osobno

Browser i Timeline robią dwie różne rzeczy.  
Użytkownik nie musi zgadywać, czy patrzy na „stan końcowy” czy „kroki budowy”.

### 19.4. Parametry nie są osobną religią

Można modelować intuicyjnie, a potem płynnie wejść w pełną parametryzację.

### 19.5. Direct edit nie jest herezją

Fusion nie traktuje operacji bezpośrednich jako czegoś wstydliwego. To część normalnej praktyki projektowej.

### 19.6. Dobre ustawienia domyślne robią ogromną część roboty

- auto-projection,
- look at sketch,
- podgląd profili,
- szybkie menu pod kursorem,
- filtry selekcji,
- sensowne zachowanie preselection.

### 19.7. To nie jest „prosty CAD”. To jest „CAD o niskim tarciu”

To fundamentalna różnica.

---

## 20. Co w FreeCAD już dziś jest dobrą bazą pod „Fusion-like” doświadczenie

Trzeba uczciwie powiedzieć: FreeCAD nie startuje z zera.

### 20.1. Mocne fundamenty, które już istnieją

1. **PartDesign Body jako sekwencja feature’ów tworzących pojedynczą bryłę** [F2]
2. **Aktywny Body** jako zalążek sensownego kontekstu pracy [F2]
3. **Sketcher z dojrzałym systemem więzów i solverem** [F3]
4. **Auto constraints i stopnie swobody** [F2] [F3]
5. **Wiele stylów nawigacji** [F4]
6. **Topological naming mitigation w FreeCAD 1.0** [F1]
7. **Usprawnienia UI w FreeCAD 1.0**
   - rotational centre indicators,
   - selection item filters,
   - on-model task panel,
   - combo view modes,
   - lepsze narzędzia pomiarowe. [F1]

### 20.2. Najważniejsza teza projektowa

> **FreeCAD ma już wystarczająco mocny „silnik”, aby zbudować doświadczenie bliższe Fusion 360.**  
> Największa robota jest w:
> - integracji,
> - ujednoliceniu,
> - skróceniu dystansu między użytkownikiem a komendą,
> - obniżeniu liczby przełączeń kontekstu.

---

## 21. Główne luki FreeCAD z punktu widzenia użytkownika Fusion

### 21.1. Fragmentacja workbenchy

Użytkownik czuje, że „przeskakuje między modułami”, podczas gdy w Fusion ma poczucie pracy w jednym środowisku Design z różnymi zakładkami.

### 21.2. Mieszanie struktury, historii i wyniku w jednym drzewie

FreeCAD jest przejrzysty obiektowo, ale mniej przejrzysty temporalnie.

### 21.3. Zbyt panelowa interakcja

Task/Combo View bywają funkcjonalnie poprawne, ale oddalone od obiektu i kursora.

### 21.4. Zbyt różne języki operacji

Pad/Pocket/Additive/Subtractive/Part Extrude/Boolean to z perspektywy mocy logiczne rozróżnienia, ale z perspektywy początkującego – nadmiar decyzji.

### 21.5. Słabszy workflow direct edit

Szczególnie przy imporcie STEP albo lokalnych poprawkach bez historii.

### 21.6. Mniejsza spójność selekcji i nawigacji

Mimo postępów, wciąż łatwo odczuć, że różne części systemu zachowują się trochę inaczej.

### 21.7. Parametry są potężne, ale nie wszędzie równie „pod ręką”

To powoduje, że wielu użytkowników korzysta z nich mniej, niż mogłoby.

---

## 22. Specyfikacja funkcjonalna dla moda FreeCAD – wymagania, które naprawdę mają sens

Poniżej daję listę wymagań tak, jakbym pisał backlog produktu.

### 22.1. Warstwa powłoki / shell

**R1. Unified Design Workbench**  
Jedna główna przestrzeń pracy z zakładkami:
- Sketch,
- Solid,
- Surface,
- Form,
- Assembly,
- Inspect.

**R2. Preselection i postselection wszędzie**  
Każda główna komenda musi działać zarówno po wcześniejszym zaznaczeniu, jak i po uruchomieniu.

**R3. In-canvas mini dialogs**  
Podstawowe polecenia nie mogą wymagać w 100% patrzenia w lewy panel.

**R4. Command palette pod `S`**  
Wyszukiwanie komend, parametrów i ostatnich działań.

**R5. Radialne menu pod prawym przyciskiem**  
Zależne od kontekstu i typu zaznaczenia.

**R6. Stała logika potwierdzania**  
`Enter` = OK, `Esc` = Cancel, prawy przycisk = menu/gest, nie chaos.

### 22.2. Kontekst i struktura

**R7. Active Modeling Context**  
Aktywny komponent/body/sketch ma być zawsze jawnie pokazany.

**R8. Tree Filtering / Browser Lens**  
Widok uproszczony tylko dla aktywnego kontekstu + przełącznik „expert tree”.

**R9. Wygaszanie geometrii spoza kontekstu**  
Podobnie do półprzezroczystości rodzeństwa w Fusion. [A19]

**R10. Nowe obiekty trafiają automatycznie do właściwego miejsca**  
Bez potrzeby ciągłego ręcznego porządkowania.

### 22.3. Sketcher UX

**R11. Floating Sketch Palette**  
Look At, Slice, Grid, Snap, Profiles, Constraints, Dimensions, Projected Geometry, 3D Sketch.

**R12. Auto project edge loops przy szkicu na ścianie**  
Z trybem linked/unlinked.

**R13. Dimensioning on the go**  
Tworzenie wymiaru natychmiast po narysowaniu geometrii.

**R14. Constraint inference preview**  
Pokazywanie sugerowanego więzu jeszcze przed kliknięciem.

**R15. DOF meter in canvas**  
Nie tylko w task panelu.

**R16. Finish Sketch w mini-toolbarze**  
Pod ręką, nie na końcu panelu.

### 22.4. Główne komendy modelowania

**R17. Unified Extrude**  
Jedno polecenie użytkowe pokrywające Add/Cut/Intersect/New Body/New Component.

**R18. Unified Revolve / Sweep / Loft**  
Spójny schemat wejścia, preview, rezultatu.

**R19. Adaptive Quick Shape Edit (Press Pull equivalent)**  
Profil -> Extrude, krawędź -> Fillet, ściana -> Offset/Move Face.

**R20. Unified Hole Tool**  
Simple/counterbore/countersink/thread.

**R21. Unified Pattern/Mirror**  
Faces/Bodies/Features/Components w jednym interfejsie.

**R22. Unified Boolean**  
Join/Cut/Common/Boundary-like workflow.

### 22.5. Direct edit

**R23. Offset Face**
**R24. Move Face**
**R25. Delete + Heal Face**
**R26. Replace Face**
**R27. Move/Copy Body**

Każda z tych komend musi:
- mieć preview,
- mieć manipulator,
- zapisywać się jako normalny feature,
- dawać się edytować po fakcie.

### 22.6. Historia

**R28. History Strip**  
Pasek historii aktywnego kontekstu.

**R29. Rollback Marker**
**R30. Suppress/Unsuppress**
**R31. Reorder where safe**
**R32. Error badges / warning badges**

### 22.7. Parametry

**R33. Every numeric field = expression field**
**R34. Inline parameter creation (`Name=Value`)**
**R35. Units everywhere**
**R36. Favorites / recent parameters**
**R37. Batch update toggle for heavy models**

### 22.8. Selekcja i nawigacja

**R38. Selection Priority presets**  
Komponent / Body / Face / Edge jako tryby priorytetowe, przełączane jawnie i widocznie.

**R39. Selection Filters**  
Filtry typów obiektów z pamięcią stanu między komendami.

**R40. Select Through**  
Możliwość świadomego wybierania geometrii ukrytej za inną geometrią.

**R41. Depth Picker**  
Lista kandydatów pod kursorem dla gęstych obszarów modelu.

**R42. Window/Crossing/Paint/Freeform**  
Cztery spójne tryby selekcji obszarowej, dostępne z klawiatury i z menu lokalnego.

**R43. Fusion-like navigation preset**  
Dokładna mapa wejścia: rolka = zoom, `MMB drag` = pan, `Shift + MMB drag` = orbit, `Shift + click + MMB` = orbit around point / reset pivotu.

**R43a. No modal camera switching**  
Kamera ma być stale dostępna w tle aktywnej komendy modelowania, bez przechodzenia do osobnego narzędzia widoku.

**R44. Orbit center indicator**  
Widoczny znacznik pivotu z opcjonalnym czasowym wygaszaniem.

**R44a. Deterministic pivot rules**  
Jawna kolejność ustalania środka obrotu: punkt wskazany -> zaznaczenie -> aktywny szkic -> extents modelu.

**R44b. Fit / Look At / Align to Selection cluster**  
Zestaw szybkich komend odzyskiwania orientacji w jednym miejscu UI.

**R45. View orientation widget**  
Widget klasy Navigation Cube / ViewCube z obsługą: ortho, perspective, isometric, fit all, fit selection, align to selection.

**R45a. Cursor-local navigation menu**  
Prawy przycisk myszy powinien dawać radialne menu z Pan / Zoom / Orbit / Fit / Look At / Isolate.

**R45b. Performance-aware navigation**  
Opcjonalne animacje przejść, minimalny FPS podczas nawigacji, tymczasowe obniżenie kosztownych efektów podczas orbity.

**R45c. Keyboard layer stays command-centric**  
Klawiatura ma priorytetowo wywoływać komendy i przełączniki UI, a nie dublować pełne sterowanie kamerą we wszystkich trybach.

### 22.9. Performance i niezawodność

**R46. Preview pipeline niezależny od pełnej przebudowy**
**R47. Szybki lokalny recompute**
**R48. Czytelne komunikaty błędów operacji**
**R49. Naprawa broken references**
**R50. Bezpieczne obchodzenie topological naming w nowych komendach**

---

## 23. Priorytety wdrożeniowe – co zrobić najpierw

### 23.1. P0 – to daje największy efekt od razu

1. Unified Design Workbench  
2. Floating Sketch Palette  
3. Unified Extrude  
4. Adaptive Quick Shape Edit  
5. Selection Filters + Priority + Depth Picker  
6. Fusion-like navigation preset  
7. Command palette + radial menu  
8. Active Modeling Context + tree filtering  
9. Inline parameters in every numeric field  
10. Offset Face / Move Face

### 23.2. P1 – to robi z systemu coś naprawdę mocnego

11. History Strip  
12. Unified Hole Tool  
13. Unified Pattern/Mirror  
14. Delete/Heal/Replace Face  
15. Better boolean / boundary workflows  
16. Surface tab integrated into same shell  
17. Better import workflow for neutral CAD

### 23.3. P2 – zaawansowanie i wow factor

18. Form/SubD workflow  
19. Partial feature recognition from imported solids  
20. Variant/configuration UX  
21. Suppressible pattern instances  
22. Expert gesture customization

---

## 24. Minimalny zestaw, który daje 80% „feelingu Fusion”

Jeżeli zespół ma mały budżet czasowy, to wystarczy zrobić te rzeczy:

1. jeden workbench `Design`,
2. w nim `Sketch`, `Solid`, `Inspect`,
3. floating palette szkicu,
4. `S` command palette,
5. radialne menu,
6. unified extrude,
7. adaptive quick edit,
8. selection priority i depth picker,
9. aktywny kontekst z filtrowaniem drzewa,
10. parametry i jednostki w polach.

To naprawdę może zrobić ogromną różnicę bez pełnej przebudowy FreeCAD.

---

## 25. Co zachować z FreeCAD i absolutnie tego nie zepsuć

FreeCAD ma przewagi, których Fusion nie ma lub nie ma w takiej formie.

### 25.1. Zachować

- **otwartość i skryptowalność**,
- **czytelność obiektowa dla ekspertów**,
- **Python/makra**,
- **możliwość pracy bez chmury**,
- **silny ekosystem workbenchy**,
- **jawność struktury dokumentu**.

### 25.2. Nie robić

- nie ukrywać ekspertowi pełnej struktury na stałe,
- nie robić agresywnego, destrukcyjnego „direct mode” kasującego historię,
- nie kopiować 1:1 brandingu, ikon i nazw Autodesk,
- nie usuwać starych workflowów – lepiej dodać warstwę „Guided / Unified UX”.

### 25.3. Lepsza strategia

**Nowy użytkownik** widzi prosty shell.  
**Ekspert** jednym przełącznikiem wraca do pełnej jawności FreeCAD.

To jest znacznie lepsze niż próba „przerobienia całego FreeCAD na Fusion”.

---

## 26. Proponowana architektura implementacyjna moda

Poniżej propozycja nie od strony geometrii, ale od strony warstwy produktu.

### 26.1. Warstwa poleceń

```text
UI Command
  -> Intent Object
      -> Context Resolver
      -> Selection Resolver
      -> Preview Builder
      -> Backend Executor (PartDesign/Part/Sketcher/Assembly)
      -> History Node
```

### 26.2. Kluczowe moduły

#### A. Context Manager
- wie, który komponent/body/sketch jest aktywny,
- decyduje, gdzie trafiają nowe obiekty,
- filtruje browser i history strip.

#### B. Selection Engine
- filtry,
- priorytety,
- depth picking,
- preselection metadata.

#### C. Preview Engine
- generuje szybki, tymczasowy podgląd,
- oddzielony od pełnego recompute dokumentu.

#### D. Command Adapters
- mapują zunifikowane komendy UI na aktualne mechanizmy FreeCAD.

#### E. History Lens
- buduje liniowy widok historii dla aktywnego kontekstu,
- bez niszczenia prawdziwego modelu danych FreeCAD.

### 26.3. Dlaczego taka architektura ma sens

Bo pozwala:
- szybko dostarczyć UX improvement bez rewolucji w kernelu,
- etapami podmieniać backendy,
- utrzymać kompatybilność z obiektami FreeCAD,
- zminimalizować ryzyko regresji.

---

## 27. Wzorzec „nie kopiuj wyglądu, kopiuj gramatykę”

To jest bardzo ważne przy projektowaniu moda.

### Fusion-like gramatyka, którą warto skopiować

- **Select first / command later** lub odwrotnie – ale zawsze spójnie
- **Preview before commit**
- **Manipulator + exact value**
- **Same result semantics in many tools**
- **Local UI in canvas**
- **Active context limits scope**
- **History is visible**
- **Parameters are allowed everywhere**
- **Direct edit is normal**
- **Navigation and selection are first-class**

### Rzeczy, których nie trzeba kopiować literalnie

- dokładny układ toolbarów,
- nazwy brandowe (`ViewCube`, `T-Spline`, itd.),
- cloud/data panel,
- wizualne detale Autodesk.

---

## 28. Przykładowy idealny workflow w przyszłym modzie FreeCAD

Poniższy scenariusz pokazuje, jak powinno to wyglądać, jeśli celem jest doświadczenie zbliżone do Fusion.

### Zadanie: zrobić wspornik z otworem i patternem

1. Użytkownik otwiera `Design`.
2. Tworzy `Component`.
3. W komponencie ma aktywne `Body`.
4. Klika `Sketch` na płaszczyźnie – kamera automatycznie patrzy na płaszczyznę, palette szkicu pojawia się przy obiekcie.
5. Rysuje profil – sugerowane więzy pojawiają się przy kursrorze, po narysowaniu od razu wpisuje wymiar.
6. Klik `Finish Sketch`.
7. `S` -> `Extrude`.
8. Wybiera `Join`, przeciąga manipulator, wpisuje dokładną wartość.
9. Na górnej ścianie `Sketch` -> okrąg -> `Finish Sketch`.
10. `Hole` albo `Quick Shape Edit`.
11. `Pattern` -> typ `Feature`, wybór osi, ilość, preview.
12. Zmiana szerokości? Wystarczy edycja wymiaru albo parametru `Width`.
13. Potrzebna szybka poprawka importowanej ściany? `Quick Shape Edit` lub `Offset Face`.
14. Na dole history strip widać wszystkie kroki.

Jeżeli FreeCAD mod pozwoli zrobić to wszystko:
- bez zmiany workbencha,
- bez częstego nurkowania w drzewie,
- bez walki z orbitą,
- z dobrym preview,
to użytkownik realnie poczuje różnicę.

---

## 29. Metryki sukcesu dla projektu

Żeby ocenić, czy mod rzeczywiście przybliża FreeCAD do doświadczenia Fusion, warto ustalić mierzalne kryteria.

### 29.1. Metryki UX

- liczba zmian workbencha przy modelowaniu prostego detalu: **0 lub 1**
- liczba wejść do drzewa podczas podstawowego workflow: **minimalna**
- 90% najczęstszych poleceń dostępnych:
  - z radialnego menu,
  - z command palette,
  - albo z preselection
- każda podstawowa operacja pokazuje preview w czasie odczuwalnie natychmiastowym
- użytkownik może wybrać ukrytą ścianę bez ręcznego chowania wielu obiektów
- każde pole liczbowe akceptuje jednostki i wyrażenia

### 29.2. Metryki techniczne

- brak niszczenia historii przy direct edit,
- brak „sierocych” obiektów po anulowaniu komend,
- poprawne osadzanie nowych szkiców/feature’ów w aktywnym kontekście,
- wysoka odporność na zmiany topologiczne,
- brak znaczącego pogorszenia kompatybilności z istniejącymi dokumentami.

---

## 30. Najważniejsza decyzja projektowa: co jest celem?

### Cel zły

„Zróbmy FreeCAD, który wygląda jak Fusion.”

### Cel dobry

„Zróbmy FreeCAD, w którym modelowanie ma:
- niski koszt wejścia,
- spójny język operacji,
- doskonałą selekcję i nawigację,
- lokalne UI,
- silne parametry,
- hybrydę direct + parametric,
- i nadal zachowuje otwartość oraz siłę FreeCAD.”

To jest znacznie lepszy i realniejszy cel.

---

## 31. Konkluzja końcowa

Fusion 360 jest przyjemny, bo:
- redukuje liczbę decyzji przed pierwszym kliknięciem,
- utrzymuje spójny język operacji,
- trzyma UI blisko geometrii,
- daje jednocześnie parametry i manipulację bezpośrednią,
- rozdziela strukturę modelu od historii jego powstawania,
- traktuje selekcję i nawigację jako krytyczny element produktu.

FreeCAD może stać się odczuwalnie podobny w kwestii modelowania **bez kopiowania całego Fusion 360**.  
Najbardziej sensowna droga to:

1. **zunifikować shell użytkownika**,  
2. **ułatwić szkicowanie i selekcję**,  
3. **ujednolicić komendy bryłowe**,  
4. **dodać direct edit jako feature’y historii**,  
5. **wyeksponować aktywny kontekst i parametry**.

Jeżeli miałbym wskazać jedno zdanie, które najlepiej streszcza cały dokument, brzmiałoby ono tak:

> **FreeCAD nie potrzebuje mniejszej złożoności geometrycznej. Potrzebuje mniejszej złożoności interakcyjnej.**

---

## 32. Załącznik A – mapa cech Fusion -> rekomendacja dla FreeCAD

| Cecha Fusion 360 | Co daje użytkownikowi | Stan w FreeCAD | Rekomendacja |
|---|---|---|---|
| Sketch Palette w canvas | krótsza droga wzroku i myszy | częściowo task panel | pływająca paleta szkicu |
| Auto-project na ścianie | szybki kontekst referencyjny | zewnętrzna geometria jest, ale mniej płynna | projekcja automatyczna linked/unlinked |
| Press Pull | niski koszt wejścia | brak spójnego odpowiednika | meta-komenda adaptacyjna |
| Jedna logika operacji (Join/Cut/New Body...) | łatwiejszy model mentalny | logika rozbita między workbenchami | zunifikowany front-end komend |
| Active component + filtered timeline | porządek | jest active body, brak pełnego odpowiednika | Active Modeling Context |
| Browser + Timeline | struktura oddzielona od czasu | drzewo miesza role | history strip + filtered browser |
| Selection priority / filters | mniej frustracji | poprawa w 1.0, ale można dużo więcej | pełny selection engine |
| Marking menu | szybki ekspert workflow | zwykłe menu kontekstowe | radial menu |
| `S` toolbox | szybkie wyszukiwanie | brak domyślnego odpowiednika | command palette |
| Inline parameters | precyzja bez ciężaru | silne expressions, słabszy UX | wyrażenia w każdym polu |
| Direct edit features | szybka korekta importów | częściowo, niespójnie | offset/move/delete/replace face |
| Multi-body per component | elastyczny workflow | Body = single solid [F2] | komponent z wieloma bodies na froncie UX |
| Body type conversions | hybryda exact + form + mesh | rozproszone | jeden shell z konwersjami |
| Navigation presets | niski koszt migracji | style już są [F4] | dodać preset Fusion-like |

---

## 33. Załącznik B – proponowany plan wdrożenia w 3 etapach

### Etap 1 – Shell i ergonomia
- Unified Design Workbench
- command palette
- radial menu
- active context
- filtered browser
- navigation preset
- selection filters/priority

### Etap 2 – Core modeling UX
- floating sketch palette
- unified extrude
- unified hole
- quick shape edit
- pattern/mirror
- in-canvas manipulators
- inline parameters

### Etap 3 – Hybrid and advanced
- history strip
- direct edit feature set
- better import workflow
- surface/form integration
- error repair tools
- advanced suppression/reorder

---

## 34. Źródła i dokumentacja

### Autodesk / Fusion 360
- **[A1]** Parametric Timeline and Features in Fusion  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Assemble/files/ASM-TIMELINE.htm
- **[A2]** Switch a design to Direct Modeling mode  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Assemble/files/ASM-DO-NOT-CAPTURE-DESIGN-HISTORY.htm
- **[A3]** Adjust Design History preference for new designs  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Assemble/files/ASM-DESIGN-HISTORY-PREFERENCE.htm
- **[A4]** Tutorial: Editing a model parametrically and using direct modeling  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/GUID-6AAFC31D-707F-46B1-997F-83D25E9EA57B.htm
- **[A5]** Create solids with Press Pull  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/GUID-02F9ADA3-7556-42A9-8AD1-552728D537AB.htm
- **[A6]** Marking menu reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-GetStarted/files/GUID-6514ABC1-CB75-4F0B-AB0E-316FAD36BA93.htm
- **[A7]** Sketch Palette reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/GUID-4183A4B7-E002-4396-AD5A-7FF3C8B2F33A.htm
- **[A8]** Project reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/GUID-6EE7B230-A280-45B7-8868-D96E4CE44B62.htm
- **[A9]** Use selection priority filters  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SELECTION-PRIORITY-FILTERS.htm
- **[A10]** Use selection filters  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SELECTION-FILTERS.htm
- **[A11]** Select objects  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SELECT-OBJECTS.htm
- **[A12]** Extrude reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-EXTRUDE.htm
- **[A13]** Mirror reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-MIRROR-DIALOG.htm
- **[A14]** Pattern reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-PATTERN.htm
- **[A15]** Holes and threads  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-HOLE-THREAD.htm
- **[A16]** Boundary Fill reference  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Patch/files/SFC-REF-BOUNDARY-FILL.htm
- **[A17]** Convert a T-Spline to a solid  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Sculpt/files/GUID-9A356F5F-38E7-4DFA-A058-7E64CC519260.htm
- **[A18]** Edit a face using T-Splines  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/GUID-1B3DAC65-5E07-4DCE-A670-C60F39BD261B.htm
- **[A19]** Activate a component in an assembly  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Assemble/files/GUID-C652E02F-0070-4DA0-A3E7-02BA9541D5A1.htm
- **[A20]** Bodies  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Assemble/files/GUID-C1AB4941-D7AD-4D27-A035-2FA9208635B6.htm
- **[A21]** Create or edit parameters  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-MODIFY-CHANGE-PARAMETERS.htm
- **[A22]** Parameter.expression Property  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/Parameter_expression.htm
- **[A23]** Imported file is missing parametric design history in Fusion  
  https://help.autodesk.com/view/fusion360/ENU/?caas=caas%2Fsfdcarticles%2Fsfdcarticles%2FImported-files-do-not-contain-timeline-features-in-Fusion-360.html
- **[A24]** Automatically constrain and dimension your sketch  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-AUTO-CONSTRAIN.htm
- **[A25]** Fully define and constrain sketches  
  https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-FULLY-DEFINE-CONSTRAIN-SKETCH.htm
- **[A26]** Fusion keyboard shortcuts reference  
  https://help.autodesk.com/view/fusion360/ENU/?guid=GUID-F0491540-0324-470A-B651-2238D0EFAC30
- **[A27]** Fusion preferences reference  
  https://help.autodesk.com/view/fusion360/ENU/?guid=GUID-878489CD-3A23-4303-8450-C2F4F8E410B1

### FreeCAD
- **[F1]** FreeCAD Version 1.0 Released  
  https://blog.freecad.org/2024/11/19/freecad-version-1-0-released/
- **[F2]** Creating a simple part with PartDesign  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Creating_a_simple_part_with_PartDesign.html
- **[F3]** Sketcher Dialog  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Sketcher_Dialog.html
- **[F4]** Mouse navigation  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Mouse_navigation.html
- **[F5]** Selection methods  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Selection_methods.html
- **[F6]** Navigation Cube  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Navigation_Cube.html
- **[F7]** Std ViewIsometric  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Std_ViewIsometric.html
- **[F8]** Std ViewFitSelection  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Std_ViewFitSelection.html
- **[F9]** Std ViewZoomIn  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Std_ViewZoomIn.html
- **[F10]** Std ToggleVisibility  
  https://reqrefusion.github.io/FreeCAD-Documentation-html/wiki/Std_ToggleVisibility.html

### Uwaga o źródłach FreeCAD
Dokumentacja FreeCAD w linkach [F2]-[F10] pochodzi z publicznego HTML mirror wiki FreeCAD.  
W praktyce jest to wygodne źródło do czytania i cytowania w formie statycznej, ale przy wdrożeniu warto sprawdzać też bieżący stan głównego repozytorium i oficjalnej wiki projektu.
