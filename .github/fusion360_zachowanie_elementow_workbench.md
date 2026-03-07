# Fusion 360 — zachowanie elementów w środowisku modelowania (selection, faces, cut, body operations)

## Cel dokumentu

Ten dokument opisuje **zachowanie elementów topologicznych i komend modelowania** w środowisku **Design > Solid** programu Autodesk Fusion 360. Skupia się na tym, **co dokładnie dzieje się po zaznaczeniu konkretnego typu obiektu** — profilu szkicu, ściany, krawędzi, bryły lub komponentu — oraz jak ten wybór wpływa na działanie operacji takich jak:

- `Extrude`
- `Press Pull`
- `Fillet`
- `Chamfer`
- `Hole`
- `Shell`
- `Draft`
- `Combine`
- `Split Face`
- `Split Body`
- `Replace Face`
- `Delete Face`
- `Move/Copy`
- `Mirror`

Dokument ma charakter **analityczny i funkcjonalny**. Celem nie jest opis marketingowy, tylko rozbicie systemu na zachowania, które można później odwzorować w FreeCAD lub w dedykowanym workbenchu inspirowanym Fusion 360.

---

## 1. Model pojęciowy: co użytkownik faktycznie zaznacza w Fusion

W Fusion 360 użytkownik nie operuje wyłącznie na „obiekcie CAD”, lecz na **różnych klasach bytów geometrycznych i logicznych**, które mają odmienną semantykę.

### 1.1 Główne typy zaznaczeń

1. **Sketch Profile** — zamknięty obszar szkicu.
   - Jest to obszar wynikający z przecięcia krzywych szkicu.
   - Nie jest bryłą ani ścianą; jest wejściem do tworzenia bryły lub wycięcia.
   - Najczęściej uruchamia `Extrude`, `Emboss`, `Hole (From Sketch)`, `Sweep`, `Loft`.

2. **Sketch Curve / Sketch Point** — elementy szkicu.
   - Mogą służyć jako ścieżki, osie, referencje lub pozycje otworów.
   - W `Hole` pozycja i orientacja są wnioskowane z punktu szkicu i normalnej szkicu.

3. **Face (BRepFace)** — ściana bryły lub powierzchni.
   - Może być płaska, cylindryczna, stożkowa, sferyczna, spline’owa.
   - Używana jako obszar do przesunięcia, usunięcia, przecięcia, zdefiniowania kierunku, oparcia szkicu, celu „To Object”, lustra lub splitu.

4. **Edge (BRepEdge)** — krawędź bryły lub powierzchni.
   - Najczęściej uruchamia `Fillet`, `Chamfer`, służy jako kierunek, oś obrotu, tor ruchu, referencja pozycjonująca.
   - Może brać udział w `Draft`, `Split Face`, `Move/Copy`, `Hole` jako referencja odległościowa.

5. **Vertex / Body Vertex** — punkt topologiczny na bryle.
   - Używany do lokalizacji, chwytów manipulatora i pozycjonowania.

6. **Body** — ciało bryłowe albo powierzchniowe.
   - Jednostka operacji typu `Combine`, `Split Body`, `Shell`, `Move/Copy`, `Mirror`.
   - Może być „targetem” albo „toolem” w operacjach boolowskich.

7. **Component** — jednostka organizacyjna/zespołowa.
   - Nowe szkice, ciała i feature’y są przypisywane do **aktywnego komponentu**.
   - Aktywacja komponentu wpływa na to, które elementy są edytowane i które feature’y są widoczne na osi czasu.

8. **Feature** — operacja w historii modelu.
   - Możliwa do wskazania jako obiekt wyboru w niektórych poleceniach, np. `Fillet`, `Chamfer`, `Mirror`.
   - Oznacza to, że Fusion potrafi traktować cechę jako wyższy poziom selekcji niż pojedynczą ścianę/krawędź.

---

## 2. Jak działa selekcja w Fusion 360

Fusion stosuje **jednocześnie trzy warstwy sterowania zaznaczeniem**:

1. **Selection Filters** — filtrują klasy obiektów.
2. **Selection Priority** — wymuszają dominujący typ zaznaczeń.
3. **Selection Modes** — określają geometrię zaznaczenia obszarowego.

### 2.1 Selection Filters

Fusion pozwala filtrować, które typy obiektów są w ogóle wybieralne. Filtry obejmują między innymi:

- Bodies
- Body Faces
- Body Edges
- Body Vertices
- Components
- Features
- Sketch Curves
- Sketch Points
- Sketch Profiles
- Work Geometry
- Mesh Bodies / Mesh Faces
- Joint Origins / Joints

Praktyczny skutek:

- użytkownik może całkowicie wyłączyć wybór ścian i zostawić tylko bryły,
- albo wyłączyć bryły i zostawić tylko krawędzie,
- albo ograniczyć wybór do profili szkicu, co bardzo zmniejsza „szum interakcyjny”.

To jest kluczowe dla ergonomii, bo w złożonych modelach znika konflikt: „chciałem wybrać profil, a złapałem ścianę”.

### 2.2 Selection Priority

Selection Priority to w praktyce **preset filtrów**, który ustawia priorytet na jeden typ obiektu:

- Component Priority
- Body Priority
- Face Priority
- Edge Priority

To nie jest tylko kosmetyka UI. W praktyce oznacza to, że użytkownik może przełączyć się z „trybu pracy na częściach” na „tryb pracy na geometrii lokalnej” bez ręcznego przepinania kilkunastu filtrów.

### 2.3 Selection Modes

Dla zaznaczania obszarowego Fusion rozróżnia:

- **Window selection**: przeciągnięcie z lewej do prawej wybiera tylko to, co w całości znajduje się wewnątrz obszaru,
- **Crossing selection**: przeciągnięcie z prawej do lewej wybiera wszystko, co zostaje przecięte obwiednią,
- **Freeform selection**: nieregularny obrys,
- **Paint Selection**: „malowanie” zaznaczenia przeciągnięciem.

Dodatkowo opcja **Select Through** pozwala uwzględnić obiekty zasłonięte; po wyłączeniu zaznaczane są tylko obiekty nieprzesłonięte w aktualnym widoku.

### 2.4 Selekcja w Browserze i w Canvasie

Fusion utrzymuje spójność selekcji między **Canvas** i **Browser**:

- wybranie obiektu w scenie podświetla go również w Browserze,
- zaznaczenie obiektu w Browserze ujawnia jego reprezentację w scenie,
- priorytety zaznaczeń dotyczą także selekcji obszarowej i ograniczają klasę obiektów podświetlanych w obu miejscach.

Dzięki temu użytkownik nie musi „zgadywać”, czy pracuje na face, body czy komponencie.

---

## 3. Reguła centralna: to typ zaznaczenia determinuje zachowanie komendy

Najważniejsza właściwość Fusion: **ta sama komenda może zmienić znaczenie w zależności od typu zaznaczonego bytu**.

Najczystszy przykład to `Press Pull`:

- zaznaczenie **Sketch Profile** → uruchamia logikę `Extrude`,
- zaznaczenie **Edge** → uruchamia logikę `Fillet`,
- zaznaczenie **Face** → uruchamia logikę `Offset Face`.

To oznacza, że użytkownik nie musi najpierw klasyfikować w głowie narzędzia. Najpierw wybiera **co chce zmienić**, a system dobiera odpowiednią rodzinę operacji. To jest jedna z głównych przyczyn, dla których Fusion odbiera się jako „przyjemny” w modelowaniu.

---

## 4. Zachowanie podstawowych elementów po zaznaczeniu

## 4.1 Zaznaczenie profilu szkicu

### Semantyka
Profil szkicu jest traktowany jako **2D input region**, z którego można wygenerować nową objętość, usunąć materiał albo zbudować cienkościenną geometrię.

### Typowe zachowania

#### `Extrude`
- Działa na zamkniętych profilach lub na ścianach.
- Gdy w projekcie jest tylko jeden widoczny profil, Fusion może zaznaczyć go automatycznie po uruchomieniu komendy.
- Typ operacji:
  - `Join`
  - `Cut`
  - `Intersect`
  - `New Body`
  - `New Component` (w odpowiednim trybie projektu)
- Zakres (`Extent`) może być:
  - `Distance`
  - `To Object`
  - `All`
- Start może być od płaszczyzny profilu albo z offsetem.

#### `Press Pull`
- Jeśli wejściem jest profil szkicu, Press Pull przełącza się do logiki `Extrude`.
- Użytkownik odczuwa to jako „jedno narzędzie do wypychania”, ale wewnętrznie to jest delegacja do innej klasy feature’a.

#### `Emboss`
- Profil szkicu jest rzutowany na ścianę bryły i może dodawać lub odejmować materiał (`Emboss` / `Deboss`).

### Istotna cecha UX
Profil jest „lekki poznawczo”: użytkownik widzi zamknięty obszar i od razu może go pchnąć w 3D. To skraca drogę od szkicu do bryły.

---

## 4.2 Zaznaczenie ściany (Face)

Ściana jest w Fusion najbardziej uniwersalnym typem elementu. Może oznaczać:

- obszar do lokalnej modyfikacji,
- granicę dla operacji „to object”,
- kierunek lub nośnik dla otworu,
- wejście do dzielenia, zastępowania, usuwania, shella, draftu,
- wejście do mirror lub do konstrukcji referencji.

### 4.2.1 `Press Pull` na ścianie

Po wskazaniu ściany `Press Pull` wywołuje zachowanie `Offset Face`.

To nie jest klasyczne wyciągnięcie szkicu. W praktyce:

- przesuwana jest wybrana ściana lub zestaw ścian,
- objętość bryły jest modyfikowana lokalnie,
- zachowywana jest topologia sąsiedztwa tak daleko, jak jest to możliwe,
- przy zaznaczeniu stycznych ścian można pracować na całym „płacie” geometrii.

To właśnie daje wrażenie „rzeźbienia bryły bez obowiązkowego wracania do szkicu”.

### 4.2.2 `Extrude` na ścianie

`Extrude` akceptuje nie tylko profile szkicu, ale także **faces**. To pozwala:

- wydłużyć istniejącą geometrię z poziomu ściany,
- traktować ścianę jako profil wejściowy dla dalszej operacji bryłowej,
- wykonywać `Join`, `Cut`, `Intersect`, `New Body` zależnie od kontekstu.

### 4.2.3 `Hole`

W trybie `At Point` użytkownik wybiera **Face/Point**:

- ściana wyznacza miejsce osadzenia otworu,
- referencje krawędziowe pozycjonują otwór względem obrzeży,
- głębokość może być `Distance`, `To`, `All`,
- typ otworu: `Simple`, `Counterbore`, `Countersink`,
- typ gwintu/wykonania: `Simple`, `Clearance`, `Tapped`, `Taper Tapped`.

### 4.2.4 `Shell`

`Shell` akceptuje:

- całe body,
- albo twarze do usunięcia.

Znaczy to, że zaznaczenie ściany może oznaczać **miejsce otwarcia pustej skorupy**. Fusion pozwala ustawić:

- typ offsetu: `Sharp Offset` albo `Rounded Offset`,
- kierunek grubości: `Inside`, `Outside`, `Both`,
- `Tangent Chain` dla stycznie połączonych ścian.

Jeżeli użytkownik zaznaczy body w Browserze, może utworzyć pustą bryłę **bez usuwania ścian**. Jeżeli wskaże ściany — shell tworzy otwory w tych miejscach.

### 4.2.5 `Draft`

`Draft` działa na ścianach bryły:

- typ `Fixed Plane` albo `Parting Line`,
- osobny wybór `Pull Direction`,
- osobny wybór ścian do draftu,
- opcjonalny `Tangent Chain`.

To oznacza, że ściana nie jest tylko obiektem „do przesunięcia”, ale także **podzbiorem powierzchni do transformacji kątowej**.

### 4.2.6 `Split Face`

`Split Face` dzieli ścianę na mniejsze ściany, bez rozcinania bryły na osobne body.

Możliwe splitting tool:

- szkic,
- face,
- workplane.

Tryby podziału:

- `Split with Surface`
- `Along Vector`
- `Closest Point`

Opcja `Extend Splitting Tool` zapewnia pełne przecięcie celu. Skutek modelarski jest ogromny: po podziale można lokalnie używać `Draft`, `Press Pull`, `Delete Face`, `Appearance` albo innych operacji tylko na fragmencie wcześniej jednej ściany.

### 4.2.7 `Replace Face`

`Replace Face` podmienia wskazane ściany innymi ścianami / powierzchniami / płaszczyznami, o ile nowa geometria przecina część. API dokumentuje:

- `sourceFaces` — ściany do podmiany,
- `targetFaces` — geometria docelowa,
- opcjonalny `isTangentChain`.

Ta operacja jest typowym narzędziem „naprawy geometrii” oraz dopasowywania kształtu bez przebudowy szkiców.

### 4.2.8 `Delete Face`

Fusion rozróżnia dwa znaczenia usuwania ścian:

1. **DeleteFaceFeature** — usuwa ściany i próbuje **zagoić** bryłę; jeśli healing się nie uda, operacja kończy się błędem.
2. **SurfaceDeleteFaceFeature** — usuwa ściany **bez healingu**, co może przekształcić bryłę w body powierzchniowe.

To ważna cecha architektury: „delete face” nie znaczy jednego. W systemie istnieje semantyka:

- „usuń i napraw bryłę”,
- „usuń i zostaw otwartą geometrię”.

---

## 4.3 Zaznaczenie krawędzi (Edge)

Krawędź w Fusion zwykle aktywuje operacje lokalnego wykończenia albo służy jako kierunek i referencja.

### 4.3.1 `Press Pull` na krawędzi

Po zaznaczeniu krawędzi `Press Pull` uruchamia logikę `Fillet`.

To jest jedna z najbardziej charakterystycznych decyzji interfejsowych Fusion:

- użytkownik nie musi przechodzić do osobnej komendy, jeżeli chce „złagodzić kant”,
- wystarczy złapać krawędź i przeciągnąć.

### 4.3.2 `Fillet`

`Fillet` akceptuje:

- edges,
- faces,
- features.

Typy filletu:

- standardowy `Fillet`,
- `Rule Fillet`,
- `Full Round Fillet`.

Parametry:

- Radius Type: `Constant`, `Chord Length`, `Variable`,
- Continuity: `G1` albo `G2`,
- `Tangent Chain`,
- Corner Type: `Rolling Ball` albo `Setback`.

Znaczenie zaznaczenia krawędzi:

- pojedyncza krawędź może rozwinąć się do całego łańcucha stycznego,
- użytkownik może budować różne selection sety w jednej operacji,
- fillet może być zdefiniowany nie tylko jako promień na krawędzi, ale jako bardziej złożona reguła na granicy powierzchni.

### 4.3.3 `Chamfer`

`Chamfer` akceptuje:

- edges,
- faces,
- features.

Tryby:

- `Equal Distance`
- `Two Distance`
- `Distance And Angle`

Dodatkowo:

- `Tangent Chain`
- Corner Type: `Chamfer`, `Miter`, `Blend`
- selection sets z różnymi parametrami w jednej operacji

Istotna cecha: Fusion traktuje chamfer jako operację podobną do filletu pod względem logiki selekcji, ale z inną geometrią wyniku.

### 4.3.4 Krawędź jako referencja

Krawędź może być także używana jako:

- kierunek w `Move/Copy` (`Pick Direction`),
- referencja pozycjonująca w `Hole`,
- parting tool albo kierunek w `Draft`,
- linia odniesienia w `Split Face`,
- oś konstrukcyjna pośrednia dla innych operacji.

---

## 4.4 Zaznaczenie bryły (Body)

Body jest obiektem wyższego poziomu niż ściana i krawędź. Na tym poziomie użytkownik nie modyfikuje lokalnego fragmentu, tylko **całą objętość** lub jej relację z innymi objętościami.

### 4.4.1 `Combine`

Klasyczna operacja boolowska na bryłach.

Fusion rozróżnia:

- `Target Body` — bryła modyfikowana,
- `Tool Bodies` — bryły narzędziowe.

Operacje:

- `Join`
- `Cut`
- `Intersect`

Opcje:

- `New Component`
- `Keep Tools`

Ważne zachowanie:

- przy `Cut` wynik zależy od relacji target/tool,
- `Keep Tools` pozwala zachować bryły narzędziowe zamiast ich „skonsumowania”,
- w operacjach cięcia udział brył może zależeć od **widoczności**.

### 4.4.2 `Split Body`

`Split Body` dzieli jedno ciało na dwa ciała.

Wejścia:

- `Body to Split`
- `Splitting Tool(s)` — może to być body, open body, plane, sketch curve albo face, byle narzędzie przecinało cel częściowo lub całkowicie.

Opcja:

- `Extend Splitting Tool(s)` — gdy narzędzie nie przecina w pełni, Fusion może je automatycznie rozszerzyć.

Semantyka:

- po operacji użytkownik dostaje osobne body,
- bryła nie jest „podzieloną ścianą”, lecz topologicznie rozdzielonymi objętościami,
- body pojawiają się osobno w Browserze i w Timeline.

### 4.4.3 `Shell`

Gdy użytkownik wybierze body zamiast ścian, `Shell` działa jako globalne wydrążenie objętości.

### 4.4.4 `Move/Copy`

`Move/Copy` dla body umożliwia:

- `Free Move`
- `Translate`
- `Rotate`
- `Point to Point`
- `Point to Position`
- `Create Copy`

Szczególnie ważne:

- `Free Move` nie zapisuje się parametrycznie w tabeli parametrów,
- body można traktować jak „luzem” przemieszczany element modelu wielobryłowego,
- ta komenda bywa używana przygotowawczo przed `Combine` albo `Split`.

### 4.4.5 `Mirror`

Dla body `Mirror` może:

- utworzyć `New Body`,
- albo `Join`, jeżeli wynik przecina oryginał.

To jest bardzo istotne rozróżnienie: mirror nie zawsze oznacza kopię — może być także sposobem domknięcia symetrycznej bryły w jedną całość.

---

## 4.5 Zaznaczenie komponentu (Component)

Komponent nie jest tylko folderem organizacyjnym. Ma wpływ na zachowanie modelowania.

### 4.5.1 Aktywny komponent

Po aktywowaniu komponentu:

- Timeline pokazuje tylko feature’y tego komponentu,
- komponenty nadrzędne i rodzeństwo stają się półprzezroczyste,
- nowe szkice, bodies i geometria konstrukcyjna trafiają do aktywnego komponentu.

To oznacza, że kontekst edycji jest częścią semantyki modelowania, a nie tylko warstwą organizacji danych.

### 4.5.2 Widoczność komponentów a operacje cięcia

Fusion jawnie dokumentuje, że w operacjach typu `Cut` (np. `Extrude`, `Hole`, `Boundary Fill`, a praktycznie także `Combine` w kontekście modyfikacji) **zestaw ciał podlegających przecięciu jest ustalany na podstawie widoczności podczas tworzenia operacji**.

To jest bardzo ważny mechanizm:

- widoczność nie służy wyłącznie prezentacji,
- widoczność bierze udział w definicji operacji,
- przy późniejszej edycji można zdecydować, czy przeliczać „Objects To Cut” automatycznie (`Auto-Select`), czy zachować pierwotny zestaw (`# Bodies`).

W sensie implementacyjnym UI i solver boolowski są tu sprzężone.

---

## 5. Cięcie (`Cut`) w Fusion — jak naprawdę działa

W Fusion „cut” nie jest jedną komendą, tylko **trybem operacji** obecnym w wielu narzędziach.

## 5.1 `Extrude` jako Cut

Przy `Extrude` użytkownik może wybrać `Operation = Cut`.

Zachowanie:

- profil lub face generuje objętość narzędziową,
- wynik jest odejmowany od wybranych/widocznych ciał,
- zakres cięcia zależy od `Extent`:
  - `Distance`
  - `To Object`
  - `All`
- `To Object` może kończyć się:
  - na wskazanej ścianie,
  - na ścianie i sąsiadujących ścianach,
  - na ciele (`To Body` lub `Through Body`).

To sprawia, że „wycinanie” w Fusion nie jest prostą operacją na odległość, ale ma bogatą semantykę końca cięcia.

## 5.2 `Hole` jako Cut

`Hole` też jest specjalizowanym cięciem:

- ma własną logikę pozycjonowania,
- własną topologię końcówki wiertła (`Flat` / `Angle`),
- własną logikę gwintu i łbów (`Counterbore`, `Countersink`),
- też korzysta z `Objects To Cut` zależnych od widoczności.

## 5.3 `Combine` jako Cut

W `Combine`, `Cut` to odjęcie objętości `Tool Bodies` od `Target Body`.

To jest ważne, bo Fusion ma dwa główne wzorce odejmowania:

1. **sketch-driven cut** (`Extrude`/`Sweep`/`Loft`/`Pipe` itd.),
2. **body-driven cut** (`Combine`).

## 5.4 `Boundary Fill` jako Cut

`Boundary Fill` tworzy komórki z przecięcia brył, płaszczyzn i powierzchni, a następnie pozwala użyć wybranych celli do operacji `Cut`, `Join`, `Intersect`, `New Body`, `New Component`.

To oznacza, że w Fusion istnieje także trzeci wzorzec odejmowania:

3. **cell-driven cut**.

---

## 6. Operacje lokalne a operacje globalne

Fusion zachowuje wyraźną różnicę pomiędzy operacjami:

### 6.1 Lokalnymi
Działają na fragmencie topologii:

- `Offset Face`
- `Press Pull` na ścianie
- `Fillet`
- `Chamfer`
- `Draft`
- `Replace Face`
- `Delete Face`
- `Split Face`

### 6.2 Globalnymi
Działają na całych ciałach lub relacjach między nimi:

- `Combine`
- `Split Body`
- `Shell` na body
- `Mirror` na body
- `Move/Copy` na body

To rozdzielenie jest czytelne dla użytkownika, bo lokalna operacja zwykle wymaga wyboru **face/edge**, a globalna **body/component**.

---

## 7. Tangent Chain — bardzo ważny mechanizm zachowania

`Tangent Chain` występuje w wielu narzędziach:

- `Fillet`
- `Chamfer`
- `Shell`
- `Draft`
- `Press Pull` / `Offset Face`
- `Emboss`
- i innych pokrewnych operacjach

### Znaczenie
Gdy `Tangent Chain` jest włączony:

- zaznaczenie jednej krawędzi lub ściany może rozszerzyć selection set o stycznie połączoną geometrię,
- selection jest bardziej „intencjonalny” niż dosłowny,
- późniejsze zmiany wcześniejszych feature’ów mogą zaktualizować łańcuch w downstream feature’ach.

To jest ważne nie tylko jako wygoda kliknięć. To **model relacji geometrycznych** w interfejsie. System rozumie, że dla użytkownika „ta zaokrąglona obręcz” jest jednym logicznym obiektem, mimo że topologicznie składa się z wielu krawędzi.

---

## 8. Selection sets w pojedynczej komendzie

W `Fillet` i `Chamfer` jedna komenda może zawierać **wiele selection setów**, każdy z własnymi parametrami.

Na przykład:

- zbiór A: promień 2 mm,
- zbiór B: promień 5 mm,
- zbiór C: promień zmienny.

To ogranicza konieczność tworzenia wielu osobnych feature’ów dla podobnych modyfikacji. Interfejs jest prostszy, ale operacja pozostaje bogata.

---

## 9. Widoczność jako część definicji operacji

Jedna z najważniejszych i często pomijanych cech Fusion:

w operacjach typu `Cut`, `Intersect`, czasem także złożonych operacjach objętościowych, **widoczność obiektów może definiować uczestników operacji**.

### Konsekwencje

- ukrycie body przed utworzeniem cięcia może wyłączyć je z udziału w operacji,
- edycja feature’a później może przeliczyć uczestników (`Auto-Select`) albo zachować pierwotny zestaw (`# Bodies`),
- Browser i solver modelowania nie są od siebie niezależne.

Dla workbencha inspirowanego Fusion jest to kluczowe: UI widoczności nie może być traktowane jako czysto graficzne.

---

## 10. Zachowanie komend — macierz skrócona

| Zaznaczony element | Typowa komenda | Co robi system |
|---|---|---|
| Sketch Profile | Extrude | Tworzy / dodaje / odejmuje objętość z profilu |
| Sketch Profile | Press Pull | Deleguje do Extrude |
| Face | Press Pull | Deleguje do Offset Face |
| Face | Extrude | Traktuje ścianę jako profil do wyciągnięcia |
| Face | Hole | Używa ściany jako miejsca osadzenia i kierunku lokalnego |
| Face | Split Face | Dzieli ścianę bez rozdzielania body |
| Face | Replace Face | Podmienia geometrię ściany ścianą/powierzchnią docelową |
| Face | Delete Face | Usuwa i leczy bryłę albo otwiera ją jako surface w zależności od wariantu |
| Edge | Press Pull | Deleguje do Fillet |
| Edge | Fillet | Zaokrągla lokalnie krawędź lub łańcuch styczny |
| Edge | Chamfer | Fazuje lokalnie krawędź lub łańcuch styczny |
| Body | Combine | Join / Cut / Intersect między bryłami |
| Body | Split Body | Rozdziela jedną bryłę na dwie |
| Body | Shell | Wydrąża całą objętość |
| Body | Move/Copy | Przemieszcza lub kopiuje całe ciało |
| Body | Mirror | Odbija jako nowe body albo join |
| Component | Activate | Zmienia kontekst modelowania i zakres timeline |

---

## 11. Wnioski implementacyjne dla workbencha typu Fusion w FreeCAD

Poniższe punkty wynikają bezpośrednio z opisanego zachowania, a nie z opinii estetycznych.

### 11.1 Selekcja musi być typowana
System musi rozróżniać co najmniej:

- profile szkicu,
- krawędzie,
- ściany,
- bryły,
- komponenty / kontenery.

Bez tego nie da się odtworzyć semantyki `Press Pull` ani logicznego doboru komend.

### 11.2 Komendy powinny być polimorficzne względem selekcji
Jedna komenda „modyfikacji bezpośredniej” powinna delegować do właściwego zachowania:

- profil → extrude,
- face → offset face / local push-pull,
- edge → fillet/chamfer.

### 11.3 Widoczność musi brać udział w operacjach cut
Jeśli workbench ma zachowywać się jak Fusion, to:

- lista ciał biorących udział w `Cut` nie może być oderwana od stanu widoczności,
- operacja powinna zapamiętywać uczestników albo umożliwiać ponowne automatyczne wyliczenie.

### 11.4 Potrzebne są selection sets i Tangent Chain
Bez tych dwóch mechanizmów modelowanie krawędzi i ścian będzie znacznie bardziej toporne.

### 11.5 Aktywny kontener modelowania musi wpływać na tworzenie obiektów
Nowe feature’y, szkice i bodies powinny trafiać do aktywnego kontenera, a nie do losowego miejsca w drzewie dokumentu.

---

## 12. Minimalny zestaw funkcji do odtworzenia zachowania Fusion

Jeżeli celem jest zbudowanie workbencha możliwie bliskiego Fusion w codziennym modelowaniu, minimalny zestaw zachowań powinien obejmować:

1. `Selection Filters`
2. `Selection Priority`
3. Window/Crossing/Freeform/Paint selection
4. `Select Through`
5. Polimorficzny `Press Pull`
6. `Extrude` z `Join/Cut/Intersect/New Body`
7. `Objects To Cut` zależne od widoczności
8. `Fillet` i `Chamfer` z `Tangent Chain`
9. `Shell` dla body i dla faces
10. `Split Face`
11. `Split Body`
12. `Combine`
13. `Move/Copy` z triadą i Point-to-Point
14. Aktywny komponent / aktywny kontener modelowania
15. Spójność zaznaczeń Canvas ↔ Browser

---

## 13. Podsumowanie

Zachowanie Fusion 360 w środowisku modelowania jest zorganizowane wokół kilku twardych zasad:

1. **Typ zaznaczenia jest pierwszorzędny** i zmienia znaczenie komendy.
2. **Selection system jest rozbudowany**, ale podany w prostych narzędziach: filtry, priorytety, tryby.
3. **Widoczność bierze udział w definicji operacji**, szczególnie dla cięć.
4. **Face/Edge/Body mają różne role semantyczne**, a UI to respektuje.
5. **Direct editing i parametric editing współistnieją**, bo wiele komend działa zarówno lokalnie na topologii, jak i w historii modelu.
6. **Jedna komenda może reprezentować rodzinę zachowań**, czego wzorcowym przykładem jest `Press Pull`.

Jeżeli chcesz odtworzyć odczucie pracy z Fusion 360, nie wystarczy dodać podobnych ikon. Trzeba odtworzyć:

- semantykę typów zaznaczenia,
- polimorfizm komend,
- udział widoczności w solverze,
- łańcuchy styczności,
- kontekst aktywnego komponentu,
- spójność Browser/Canvas.

To właśnie te mechanizmy składają się na zachowanie, które użytkownik odbiera jako „intuicyjne”.

---

## Źródła

1. Autodesk Fusion Help — Use selection filters  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SELECTION-FILTERS.htm

2. Autodesk Fusion Help — Use selection priority filters  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SELECTION-PRIORITY-FILTERS.htm

3. Autodesk Fusion Help — Select multiple objects with selection modes  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SELECTION-MODES.htm

4. Autodesk Fusion Help — Create solids with Press Pull  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/GUID-02F9ADA3-7556-42A9-8AD1-552728D537AB.htm

5. Autodesk Fusion Help — Extrude reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-EXTRUDE.htm

6. Autodesk Fusion Help — Hole reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/GUID-3A76B269-8C8D-437B-8F4A-85D0B2BBA492.htm

7. Autodesk Fusion Help — Fillet reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-FILLET.htm

8. Autodesk Fusion Help — Chamfer reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-CHAMFER.htm

9. Autodesk Fusion Help — Shell reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-SHELL.htm

10. Autodesk Fusion Help — Draft reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-DRAFT.htm

11. Autodesk Fusion Help — Divide a face into multiple faces  
   https://help.autodesk.com/view/fusion360/ENU/?contextId=MODEL-SPLIT-FACE-CMD

12. Autodesk Fusion Help — Split a solid body  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-SPLIT-BODY-SOLID.htm

13. Autodesk Fusion Help — Combine solid bodies  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-COMBINE.htm

14. Autodesk Fusion Help — Move/Copy reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/GUID-FFD25CD3-0707-429E-B0E6-B7F9984CDC4C.htm

15. Autodesk Fusion Help — Mirror reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Model/files/SLD-REF-MIRROR-DIALOG.htm

16. Autodesk Fusion Help — Boundary Fill reference  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Patch/files/SFC-REF-BOUNDARY-FILL.htm

17. Autodesk Fusion Help — Activate a component in an assembly  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-Assemble/files/GUID-C652E02F-0070-4DA0-A3E7-02BA9541D5A1.htm

18. Autodesk Fusion API Help — ReplaceFaceFeatures.createInput  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/ReplaceFaceFeatures_createInput.htm

19. Autodesk Fusion API Help — DeleteFaceFeatures / SurfaceDeleteFaceFeatures  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/DeleteFaceFeatures.htm  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/SurfaceDeleteFaceFeatures.htm

20. Autodesk Fusion API Help — OffsetFacesFeature / OffsetFacesFeatures  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/OffsetFacesFeature.htm  
   https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/OffsetFacesFeatures.htm
