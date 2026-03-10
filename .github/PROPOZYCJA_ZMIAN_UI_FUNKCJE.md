# UniCAD — Propozycja zmian interfejsu, wyglądu i funkcji

**Data:** 2026-03-09  
**Podstawa:** Analiza Fusion 360 vs FreeCAD (4 dokumenty analityczne)  
**Stan projektu:** Zaczątki komponentów FusionUI (FusionUIManager, FusionTabToolbar, FusionTimeline, FusionMarkingMenu, FusionNavigationBar, FusionLog) już wdrożone w `src/Gui/`

---

## Streszczenie

Propozycja obejmuje **63 konkretne zmiany** pogrupowane w 10 obszarów, z podziałem na 3 priorytety wdrożeniowe. Kluczowa zasada projektowa:

> **Nie kopiować wyglądu Fusion 360 — kopiować gramatykę pracy: wybierz → podgląd → manipuluj → potwierdź → edytuj później.**

UniCAD ma już silny backend (PartDesign, Sketcher, OpenCASCADE) i zaczątki warstwy Fusion UI. Główna praca polega na: **ujednoliceniu interakcji, skróceniu dystansu kursor↔komenda i obniżeniu liczby decyzji przed pierwszym kliknięciem.**

---

## I. INTERFEJS — Układ okna i nawigacja po UI

### Obecny stan
- `FusionTabToolbar` już istnieje — obsługuje zakładki PartDesign/Part, generyczne fallbacki
- `FusionNavigationBar` — bazowy pasek nawigacyjny na dole
- `FusionUIManager` — singleton zarządzający trybem Fusion (domyślnie wyłączony)

### Proponowane zmiany

#### 1.1 Zunifikowana powłoka „Design Workspace" [P0]
**Problem:** Użytkownik musi przeskakiwać między workbenchami (PartDesign ↔ Part ↔ Sketcher ↔ Surface).  
**Zmiana:**  
- Rozbudować `FusionTabToolbar` o stały zestaw zakładek: **SKETCH, SOLID, SURFACE, SHEET METAL, MESH, INSPECT, TOOLS**
- Zakładki nie zmieniają workbencha — organizują komendy z wielu workbenchy w logiczne grupy
- Dodać dropdown „Workspace" (Design / Manufacture / Drawing) po lewej stronie zakładek

#### 1.2 Kontekstowe zakładki z wyróżnieniem trybu [P0]
**Problem:** Użytkownik nie wie, że jest w trybie szkicu.  
**Zmiana:**  
- Przy wejściu w szkic: zakładka SKETCH pojawia się z wizualnym akcentem (niebieski highlight) + przycisk „Finish Sketch"
- Przy wejściu w Form/SubD: zakładki zastępowane przez środowisko kontekstowe + „Finish Form"
- Wzorowane na Fusion: `Contextual Tab` vs `Contextual Environment`

#### 1.3 Application Bar (QAT) — stała strefa statusowa [P1]
**Problem:** Elementy statusowe (save, undo/redo, notifications) nie mają stałego miejsca.  
**Zmiana:**  
- Dodać wąski pasek nad zakładkami: lewa strona = Save, Undo, Redo, otwarte karty dokumentów
- Prawa strona = Job Status, Notification Center, Help, profil/ustawienia
- Niezależny od kontekstu — zawsze widoczny

#### 1.4 Data Panel — lewa nakładka projektów [P2]
**Problem:** Brak spójnego panelu zarządzania plikami/projektami.  
**Zmiana:**  
- Lewy panel z nawigacją po plikach, ostatnich dokumentach i importach
- Skrót: `Ctrl+Alt+P`
- Oddzielone wizualnie (osobne tło) od Browsera modelu

---

## II. WYGLĄD — Styl wizualny i motyw

### 2.1 Dwupoziomowy system ikon [P1]
**Problem:** Brak spójności ikon między workbenchami.  
**Zmiana:**  
- Ikony 32×32 (Panel/Toolbar) — pełne detale, izometryczne bryły z cieniowaniem
- Ikony 16×16 (Dropdown, listy) — uproszczone, ten sam „DNA" linii
- Stała grubość linii (1px ramka) w obu rozmiarach
- Spójny styl: ikony geometryczne = izometria z cieniem; ikony systemowe = piktogramy płaskie

### 2.2 Motywy kolorystyczne [P1]
**Problem:** Brak motywu zbliżonego do Fusion.  
**Zmiana:**  
- Dodać motyw „UniCAD Light": tło paneli `#F0F0F0`, canvas `#FFFFFF`, akcent `#0696D7`
- Dodać motyw „UniCAD Dark": tło paneli `#374150`, canvas `#465564`, akcent `#6ECDFA`
- Opcja „Match OS" (auto-detekcja ciemnego/jasnego motywu systemu)
- Zachować obecne motywy FreeCAD jako alternatywne

### 2.3 Warstwowe tooltipy [P1]
**Problem:** Tooltipy nie dają wystarczającej informacji.  
**Zmiana:**  
- Warstwa 1 (natychmiastowa, <0.3s): nazwa + skrót klawiszowy
- Warstwa 2 (po ~1s): dłuższy opis + miniatura/ikona
- Warstwa 3 (opcjonalnie, po ~2s): animowany clip / link do pomocy
- Maksymalna szerokość: 300px

### 2.4 Wygaszanie kontekstu wizualnego [P0]
**Problem:** Przy pracy w jednym komponencie inne elementy rozpraszają.  
**Zmiana:**  
- Aktywny komponent: pełna nieprzezroczystość
- Rodzeństwo i rodzic: półprzezroczyste (opacity ~30%)
- Elementy spoza aktywnego Body: stonowane kolory
- Jasne wizualne wyróżnienie „co jest aktywne" w nagłówku 3D view

---

## III. SZKIC (Sketcher UX) — centrum modelowania

### 3.1 Pływająca paleta szkicu w canvas [P0]
**Problem:** Opcje szkicu tylko w odległym task panelu.  
**Zmiana:**  
- Lekki pływający panel w 3D view przy aktywnym szkicu, zawierający:
  - `Look At` (orientacja kamery na płaszczyznę)
  - `Slice` (tymczasowy przekrój brył)
  - Siatka/Snap — on/off
  - Highlight zamkniętych profili — on/off
  - Więzy/Wymiary — widoczność
  - Geometria rzutowana — widoczność
  - Przełącznik 3D Sketch
- Panel minimalizowalny, dokowany lub pływający

### 3.2 Auto-project krawędzi przy szkicu na ścianie [P1]
**Problem:** Ręczne rzutowanie krawędzi ściany to dodatkowe kroki.  
**Zmiana:**  
- Przy tworzeniu szkicu na istniejącej ścianie: automatyczne rzutowanie pętli krawędzi
- Tryby: `Linked` (powiązane, aktualizują się) / `Unlinked` (jednorazowa kopia)
- Wizualne odróżnienie: lined = niebieskie, unlinked = zielone

### 3.3 DOF meter w obszarze roboczym [P0]
**Problem:** Informacja o stopniach swobody ukryta w task panelu.  
**Zmiana:**  
- Pasek/badge w canvas: „Fully Constrained" / „3 DOF remaining"
- Kliknięcie na badge: podświetlenie niedowiązanych elementów
- Nadmiarowe więzy: badge ostrzegawczy + podświetlenie na czerwono

### 3.4 Dimensioning on the go [P0]
**Problem:** Wymiarowanie wymaga osobnego narzędzia po narysowaniu geometrii.  
**Zmiana:**  
- Po narysowaniu linii/okręgu: mały input wymiaru pojawia się obok kursora
- `Enter` — zatwierdza i kontynuuje rysowanie
- `Tab` — przeskakuje do następnego pola (szerokość → wysokość)
- `Esc` — anuluje wymiar, geometria zostaje

### 3.5 Inferencja i podgląd więzów [P1]
**Problem:** Sugestie więzów mogą zaskoczyć użytkownika.  
**Zmiana:**  
- Wyświetlanie sugerowanego więzu (np. ikona „tangent" / „coincident") przy kursorze **przed** kliknięciem
- Przezroczysty badge z typem więzu
- Możliwość odrzucenia sugestii klawiszem (np. `X`)

### 3.6 Finish Sketch w mini-toolbarze [P0]
**Problem:** „Finish Sketch" jest daleko, na końcu task panelu.  
**Zmiana:**  
- Mały pływający przycisk „Finish Sketch" w prawym górnym rogu canvas (lub w pobliżu szkicu)
- Dostępny też pod prawym przyciskiem (Marking Menu, pozycja N)
- Wizualnie wyróżniony (niebieski, jak w Fusion)

---

## IV. KOMENDY MODELOWANIA — ujednolicenie

### 4.1 Unified Extrude [P0]
**Problem:** W FreeCAD osobno Pad, Pocket, Part Extrude, Additive/Subtractive.  
**Zmiana:**  
- Jedna komenda `Extrude` (`E` jako skrót), która:
  - Przyjmuje profil szkicu **lub** ścianę bryły
  - Kierunek: One Side / Two Sides / Symmetric
  - Zakres: Distance / Up to Face / Through All / Up to Next
  - Rezultat: **Add** / **Cut** / **Intersect** / **New Body** / **New Component**
  - Taper angle
  - Thin Extrude
- Wewnętrznie dispatchuje do PartDesign::Pad lub PartDesign::Pocket
- Zawsze aktywny podgląd 3D
- Manipulator przeciągania + dokładne pole liczbowe

### 4.2 Adaptive Quick Shape Edit (odpowiednik Press Pull) [P0]
**Problem:** Użytkownik nie wie, jakie narzędzie uruchomić.  
**Zmiana:**  
- Jedna meta-komenda (`Q` jako skrót), która na podstawie zaznaczenia:
  - **Profil szkicu** → Unified Extrude
  - **Krawędź bryły** → Fillet (domyślnie) / Chamfer (z modyfikatorem Shift)
  - **Ściana bryły** → Offset Face / Move Face
- W historii powstaje **kanoniczny feature** (Extrude, Fillet, OffsetFace), nie „QuickEditFeature"
- Zawsze manipulator + value input

### 4.3 Unified Revolve / Sweep / Loft [P1]
**Problem:** Różne dialogi, różna terminologia.  
**Zmiana:**  
- Spójny schemat dialogów: wejścia → preview → rezultat (ten sam słownik: Add/Cut/Intersect/New Body)
- Loft: sekcje + opcjonalne rails + centerline w jednym panelu
- Sweep: profil + ścieżka + orientacja + continuity
- Revolve: profil + oś + kąt + rezultat

### 4.4 Unified Hole Tool [P1]
**Problem:** Brak dedykowanego narzędzia otworów z semantyką technologiczną.  
**Zmiana:**  
- Jedno narzędzie `Hole` (`H` jako skrót) z opcjami:
  - Simple / Counterbore / Countersink
  - Thread: None / Modeled / Cosmetic
  - Depth: Distance / Through All / To Face
  - Pozycjonowanie: punkt szkicu / krawędź + offset
- Panel do wyboru standardów gwintów (ISO, UNC, UNF...)

### 4.5 Unified Pattern / Mirror [P1]
**Problem:** W FreeCAD kilka oddzielnych narzędzi.  
**Zmiana:**  
- Jeden interfejs Pattern/Mirror akceptujący:
  - Faces / Bodies / Features / Components
  - Linear Pattern: oś + ilość + rozstaw
  - Circular Pattern: oś + ilość + kąt
  - Mirror: płaszczyzna → wynik Join lub New Body
  - Suppress poszczególnych instancji

### 4.6 Unified Boolean [P1]
**Problem:** Rozbite operacje boolowskie.  
**Zmiana:**  
- Jeden panel Boolean/Combine: Target Body + Tool Bodies
- Operacje: Join / Cut / Intersect
- Opcje: Keep Tools / New Component
- Smart auto-select (widoczne body) z jawnym panelem uczestników
- Przełącznik: Auto / Zamrożona lista

---

## V. DIRECT EDIT — edycja bezpośrednia jako feature historii

### 5.1 Offset Face [P0]
**Zmiana:** Feature parametryczny — zaznaczenie ściany → przeciągnięcie → nowy feature w historii. Tangent chain, preview, manipulator.

### 5.2 Move Face [P0]
**Zmiana:** Przesunięcie/obrót wybranych ścian bryły. Manipulator 3D + dokładne wartości. Normalny feature w historii.

### 5.3 Delete + Heal Face [P1]
**Zmiana:** Usunięcie ściany i próba automatycznego „zagojenia" bryły. Dwa tryby: z healingiem (utrzymanie solid) / bez healingu (otwarcie surface).

### 5.4 Replace Face [P1]
**Zmiana:** Podmiana ścian innymi powierzchniami/płaszczyznami. Tangent chain, target faces, source faces.

### 5.5 Move/Copy Body [P1]
**Zmiana:** Free Move / Translate / Rotate / Point to Point. Opcja Create Copy.

### 5.6 Kluczowa zasada: NIE kopiować destrukcyjnego trybu Fusion [—]
**Decyzja:** Direct edit NIE kasuje historii. Każda operacja direct edit to normalny feature w drzewie, z możliwością rollback/edit/suppress. To jest **lepsze** niż rozwiązanie Fusion, gdzie przejście w Direct Mode niszczy timeline.

---

## VI. SELEKCJA — system zaznaczania

### 6.1 Globalny pasek Selection [P0]
**Zmiana:**  
- Pasek/sekcja w UI z przyciskami priorytetu: **Component** / **Body** / **Face** / **Edge** / **Vertex**
- Kliknięcie priorytetu = preset filtrów (jak w Fusion: Selection Priority)
- Widoczny aktualny tryb priorytetu

### 6.2 Filtry selekcji [P0]
**Zmiana:**  
- Checkboksowe filtry: Bodies, Faces, Edges, Vertices, Sketch Profiles, Sketch Curves, Features, Components, Work Geometry
- Pamięć stanu filtrów między komendami
- Szybki reset do „All"

### 6.3 Select Through [P0]
**Zmiana:**  
- Przełącznik: select through on/off
- Gdy off: wybierane tylko widoczne (nieprzesłonięte) obiekty
- Gdy on: wybór obiektów „za" innymi

### 6.4 Depth Picker [P0]
**Zmiana:**  
- Po przytrzymaniu kliknięcia (lub specjalnym geście): lista obiektów pod kursorem
- Rozróżnienie: Face / Edge / Body / Feature / Component
- Eliminuje problem „chciałem wybrać ścianę, a złapałem krawędź"

### 6.5 Tryby selekcji obszarowej [P1]
**Zmiana:**  
- Window selection (L→R): tylko w pełni zawarte obiekty
- Crossing selection (R→L): także częściowo przecięte
- Freeform selection: nieregularny obrys
- Paint selection: malowanie zaznaczenia
- Klawisze `1`, `2`, `3` do szybkiego przełączania

### 6.6 Spójność Canvas ↔ Browser [P0]
**Zmiana:**  
- Zaznaczenie w canvas = podświetlenie w Browserze i odwrotnie
- Priorytety selekcji wpływają na oba widoki jednocześnie

---

## VII. NAWIGACJA — kamera i orientacja

### 7.1 Preset nawigacji „UniCAD/Fusion" [P0]
**Zmiana:**  
- Nowy styl nawigacji o dokładnym mapowaniu:
  - `MMB drag` = pan
  - Rolka = zoom
  - `Shift + MMB drag` = orbit
  - `Shift + click + MMB` = orbit around point (reset pivotu)
- Dodany jako opcja w preferencjach, **nie** zastępuje istniejących stylów

### 7.2 Reguły pivotu [P0]
**Zmiana:**  
- Deterministyczna kolejność źródeł punktu obrotu:
  1. Punkt wskazany explicite (orbit around point)
  2. Centrum zaznaczenia
  3. Centrum aktywnej płaszczyzny szkicu
  4. Centrum extents modelu
- Widoczny wskaźnik pivotu (z czasowym wygaszaniem)

### 7.3 Lokalne narzędzia odzyskiwania widoku [P0]
**Zmiana:**  
- `F6` lub `F` = Fit All
- `Look At active sketch plane` — automatyczne przy wejściu w szkic
- `Slice at sketch plane` — z pływającej palety szkicu
- `Isolate / Unisolate` — prawy przycisk → Marking Menu
- `Align to Selection`

### 7.4 Warstwa wydajnościowa nawigacji [P2]
**Zmiana:**  
- Animacje przejść widoku (smooth transition)
- Minimalny FPS podczas nawigacji
- Opcjonalne obniżenie jakości efektów podczas orbity
- Ustawienia: Performance / Quality / Custom

### 7.5 Rozdział roli: mysz = kamera, klawiatura = komendy [P0]
**Zmiana:**  
- W presecie UniCAD/Fusion: kamera sterowana myszą
- Klawiatura rezerwowana dla:
  - Skróty poleceń: `E` Extrude, `H` Hole, `Q` Quick Edit, `F` Fillet, `M` Move, `L` Line, `R` Rectangle, `C` Circle, `D` Dimension, `T` Trim, `O` Offset
  - Przełączniki UI: `S` Command Palette, `X` Construction mode
  - Selekcja: `1` Window, `2` Freeform, `3` Paint

---

## VIII. MARKING MENU i COMMAND PALETTE — przyspieszacze workflow

### 8.1 Rozbudowa FusionMarkingMenu [P0]
**Obecny stan:** Bazowa implementacja z 8 pozycjami radialnymi (`FusionMarkingMenu`).  
**Zmiana:**  
- Kontekstowość: zawartość zmienia się w zależności od:
  - Aktywnego workspace (Design/Manufacture/Drawing)
  - Aktywnej zakładki (Solid/Surface/Sketch)
  - Typu zaznaczenia (face → inne opcje niż edge)
- Domyślny układ Design/Solid:
  - N: Repeat Last Command
  - NE: Extrude / Press Pull
  - E: Sketch
  - SE: Move/Copy
  - S: More... (overflow menu)
  - SW: Fillet
  - W: Hole
  - NW: Delete
- Dolna sekcja: Pan / Zoom / Orbit / Fit / Isolate + Saved Shortcuts
- Obsługa gestów: szybkie przeciągnięcie w kierunku komendy bez czekania na pełne wyświetlenie

### 8.2 Command Palette pod `S` [P0]
**Problem:** Brak szybkiego wyszukiwania komend w jednym miejscu.  
**Zmiana:**  
- Popup przy kursorze: pole wyszukiwania + siatka ikon najczęściej używanych
- Wyszukiwanie po nazwie polecenia i po aliasach
- Możliwość przypinania ulubionych (pin/unpin)
- Sekcja „Recent Commands"
- Kontekstowe — wyniki filtrowane wg aktywnego workspace'u

### 8.3 Stała logika potwierdzania [P0]
**Problem:** Niespójna obsługa Enter/Esc między komendami.  
**Zmiana:**  
- `Enter` = OK / Apply we wszystkich dialogach
- `Esc` = Cancel we wszystkich dialogach
- Prawy przycisk = Marking Menu (nie tradycyjne menu kontekstowe)
- Double-click na feature w drzewie/timeline = Edit

---

## IX. KONTEKST i STRUKTURA — Browser, historia, parametry

### 9.1 Active Modeling Context [P0]
**Problem:** Nie zawsze jasne, do którego body/komponentu „trafiają" nowe obiekty.  
**Zmiana:**  
- Zawsze widoczny wskaźnik aktywnego kontekstu (np. breadcrumb: `Assembly > Component1 > Body`)
- Aktywacja komponentu = filtrowanie drzewa i timeline
- Nowe szkice/features trafiają automatycznie do aktywnego kontekstu

### 9.2 Tree Filtering / Browser Lens [P0]
**Problem:** Drzewo FreeCAD miesza strukturę, historię i wynik.  
**Zmiana:**  
- Tryb Simplified: pokazuje tylko elementy aktywnego kontekstu w logicznej kolejności
- Tryb Expert: pełne drzewo FreeCAD (obecne zachowanie)
- Przełącznik: przycisk w nagłówku Browsera

### 9.3 Rozbudowa FusionTimeline (History Strip) [P0]
**Obecny stan:** `FusionTimeline` wyświetla feature chips, bazowy rollback.  
**Zmiana:**  
- Rollback marker: przeciągalny wskaźnik „do którego kroku przeliczać model"
- Suppress/Unsuppress: prawy przycisk na chipie → disable/enable feature
- Reorder: drag & drop chipów tam, gdzie zależności pozwalają
- Error/warning badges: czerwony / żółty znacznik na chipie z błędem
- Filtrowanie po aktywnym komponencie: timeline pokazuje tylko cechy aktywnego kontekstu
- Double-click na chipie: otwiera edycję parametrów feature'a

### 9.4 Parametry i wyrażenia w każdym polu [P0]
**Problem:** Silnik wyrażeń FreeCAD jest potężny, ale niedostatecznie dostępny.  
**Zmiana:**  
- Każde pole liczbowe w zunifikowanych komendach akceptuje:
  - Liczbę (`50`)
  - Liczbę z jednostką (`50 mm`, `2 in`)
  - Wyrażenie (`Width * 2`)
  - Istniejący parametr (autocomplete)
  - Definicję nowego parametru: `MyWidth = 50`
- Mały przycisk `fx` przy każdym polu — rozwija edytor wyrażeń
- Autouzupełnianie parametrów z listy
- Panel ulubionych/ostatnich parametrów

### 9.5 Uproszczony model Browser [P1]
**Problem:** Rozróżnienia Document/Part/Body/Link nieintuicyjne dla nowych użytkowników.  
**Zmiana:**  
- W trybie Simplified Browser:
  - **Component** = użytkowy kontener (mapuje na App::Part)
  - **Bodies** = bryły w komponencie (mapuje na PartDesign::Body)
  - **Features** = cechy modelowania
  - **Sketches** = szkice
  - **Construction** = datum planes/axes/points
- W trybie Expert: pełna jawność FreeCAD

---

## X. WYDAJNOŚĆ i NIEZAWODNOŚĆ

### 10.1 Preview pipeline niezależny od pełnej przebudowy [P1]
**Zmiana:** Podgląd operacji (extrude, fillet...) generowany lokalnie, bez pełnego recompute dokumentu. Szybszy feedback wizualny.

### 10.2 Czytelne komunikaty błędów [P1]
**Zmiana:**  
- Gdy operacja (fillet, shell, boolean) nie może być rozwiązana: jasny opis problemu w języku użytkownika
- Badge na timeline chipie + tooltip z opisem
- Link „Show problem area" — podświetlenie geometrii powodującej błąd

### 10.3 Bezpieczne obchodzenie topological naming [P0]
**Zmiana:** Nowe zunifikowane komendy muszą korzystać z mechanizmów TNP mitigation z FreeCAD 1.0. Referencje na twarze/krawędzie powinny być odporne na przebudowę modelu.

---

## Priorytety wdrożeniowe — Plan 3 etapów

### ETAP 1 (P0) — Shell, ergonomia, rdzeń interakcji
*Efekt: użytkownik odczuwa fundamentalną zmianę w sposobie pracy*

| # | Zmiana | Sekcja |
|---|---|---|
| 1 | Zunifikowana powłoka Design Workspace + zakładki | I.1.1 |
| 2 | Kontekstowe zakładki (Sketch highlight, Finish) | I.1.2 |
| 3 | Wygaszanie kontekstu wizualnego | II.2.4 |
| 4 | Pływająca paleta szkicu | III.3.1 |
| 5 | DOF meter w canvas | III.3.3 |
| 6 | Dimensioning on the go | III.3.4 |
| 7 | Finish Sketch w mini-toolbarze | III.3.6 |
| 8 | Unified Extrude | IV.4.1 |
| 9 | Adaptive Quick Shape Edit | IV.4.2 |
| 10 | Offset Face | V.5.1 |
| 11 | Move Face | V.5.2 |
| 12 | Globalny pasek Selection + filtry | VI.6.1-6.2 |
| 13 | Select Through + Depth Picker | VI.6.3-6.4 |
| 14 | Spójność Canvas ↔ Browser | VI.6.6 |
| 15 | Preset nawigacji UniCAD/Fusion | VII.7.1 |
| 16 | Reguły pivotu | VII.7.2 |
| 17 | Narzędzia odzyskiwania widoku | VII.7.3 |
| 18 | Rozdział mysz/klawiatura | VII.7.5 |
| 19 | Rozbudowa FusionMarkingMenu (kontekstowość) | VIII.8.1 |
| 20 | Command Palette pod S | VIII.8.2 |
| 21 | Stała logika potwierdzania | VIII.8.3 |
| 22 | Active Modeling Context | IX.9.1 |
| 23 | Tree Filtering / Browser Lens | IX.9.2 |
| 24 | Rozbudowa FusionTimeline | IX.9.3 |
| 25 | Parametry i wyrażenia w każdym polu | IX.9.4 |
| 26 | Bezpieczne obchodzenie TNP | X.10.3 |

### ETAP 2 (P1) — Pełny zestaw narzędzi modelowania
*Efekt: system jest konkurencyjny funkcjonalnie*

| # | Zmiana | Sekcja |
|---|---|---|
| 27 | Application Bar (QAT) | I.1.3 |
| 28 | Dwupoziomowy system ikon | II.2.1 |
| 29 | Motywy kolorystyczne | II.2.2 |
| 30 | Warstwowe tooltipy | II.2.3 |
| 31 | Auto-project krawędzi | III.3.2 |
| 32 | Inferencja i podgląd więzów | III.3.5 |
| 33 | Unified Revolve / Sweep / Loft | IV.4.3 |
| 34 | Unified Hole Tool | IV.4.4 |
| 35 | Unified Pattern / Mirror | IV.4.5 |
| 36 | Unified Boolean | IV.4.6 |
| 37 | Delete + Heal Face | V.5.3 |
| 38 | Replace Face | V.5.4 |
| 39 | Move/Copy Body | V.5.5 |
| 40 | Tryby selekcji obszarowej | VI.6.5 |
| 41 | Uproszczony model Browser | IX.9.5 |
| 42 | Preview pipeline | X.10.1 |
| 43 | Czytelne komunikaty błędów | X.10.2 |

### ETAP 3 (P2) — Zaawansowanie i wow factor
*Efekt: system wyróżnia się na tle komercyjnych CAD-ów*

| # | Zmiana | Sekcja |
|---|---|---|
| 44 | Data Panel | I.1.4 |
| 45 | Warstwa wydajnościowa nawigacji | VII.7.4 |
| 46 | Form/SubD workflow (subdivision modeling) | — |
| 47 | Rozpoznawanie feature'ów z importu STEP | — |
| 48 | Warianty/konfiguracje UX | — |
| 49 | Suppressible pattern instances | — |
| 50 | Zaawansowana customizacja gestów | — |

---

## Minimalny zestaw dający 80% „feelingu Fusion"

Jeśli budżet jest ograniczony, te **10 zmian** daje największy efekt:

1. **Zunifikowany Design Workspace** z zakładkami (bez zmiany workbencha)
2. **Pływająca paleta szkicu** (Look At, Slice, DOF, Finish Sketch)
3. **Unified Extrude** (Add/Cut/Intersect/New Body w jednym)
4. **Quick Shape Edit** (profil→extrude, krawędź→fillet, ściana→offset)
5. **Selection Priority + Depth Picker**
6. **Preset nawigacji Fusion-like** (MMB=pan, Shift+MMB=orbit)
7. **Command Palette** pod `S`
8. **Marking Menu** kontekstowe pod prawym przyciskiem
9. **Active Modeling Context** z filtrowaniem drzewa
10. **Parametry/wyrażenia** w każdym polu liczbowym

---

## Czego NIE robić

| Anty-wzorzec | Dlaczego nie |
|---|---|
| Destrukcyjny Direct Mode kasujący historię | Lepszy wzorzec: direct edit = normalny feature historii |
| Kopiowanie 1:1 ikon/brandingu Autodesk | Naruszenie praw autorskich; lepszy własny spójny styl |
| Usuwanie starych workflowów FreeCAD | Ekosystem workbenchy to wartość; dodawać warstwę, nie kasować |
| Ukrywanie ekspertowi pełnej struktury na stałe | Zawsze przełącznik Expert/Simplified |
| Agresywna zmiana domyślnych stylów nawigacji | Dodać nowy preset, nie zastępować istniejących |
| Data Panel oparty na chmurze | Zachować offline-first filozofię FreeCAD |

---

## Architektura implementacji

```
UI Command (Unified Extrude / Quick Shape Edit / ...)
  → Intent Object (co użytkownik chce zrobić)
      → Context Resolver (aktywny komponent/body/szkic)
      → Selection Resolver (filtry, priorytety, depth picking)  
      → Preview Builder (szybki tymczasowy podgląd)
      → Backend Executor (PartDesign::Pad / Pocket / Part::Fillet / ...)
      → History Node (feature w drzewie z pełną edytowalnością)
```

### Kluczowe moduły do zbudowania/rozbudowania

| Moduł | Istniejący stan | Co dodać |
|---|---|---|
| **FusionUIManager** | ✅ Singleton, toggle on/off | Context awareness, bardziej rozbudowana orchestracja |
| **FusionTabToolbar** | ✅ Bazowe zakładki | Stałe zakładki Design, kontekstowe Sketch/Form, dropdown Workspace |
| **FusionTimeline** | ✅ Feature chips, bazowy | Rollback marker, suppress, reorder, error badges, filtrowanie kontekstowe |
| **FusionMarkingMenu** | ✅ 8 pozycji radialnych | Kontekstowość, gesty, dolny overflow, menu nawigacyjne |
| **FusionNavigationBar** | ✅ Bazowy | Fit, Look At, Slice, Isolate, Display Settings |
| **FusionLog** | ✅ Logging system | Wystarczający — rozbudowywać w miarę potrzeb |
| **Context Manager** | ❌ Brak | Aktywny komponent/body/sketch, filtrowanie drzewa, routing nowych obiektów |
| **Selection Engine** | ❌ Częściowo (FreeCAD 1.0 filtery) | Priorytety, depth picking, preselection metadata, tryby obszarowe |
| **Preview Engine** | ❌ Brak dedykowanego | Szybki podgląd bez pełnego recompute |
| **Command Adapters** | ❌ Brak | Mapowanie zunifikowanych komend na istniejące mechanizmy FreeCAD |
| **Command Palette** | ❌ Brak | Wyszukiwarka + siatka ulubionych komend |

---

## Metryki sukcesu

### UX
- Zmian workbencha przy modelowaniu prostego detalu: **0**
- Wejść do drzewa podczas podstawowego workflow: **minimalna**
- 90% najczęstszych poleceń dostępne z: Marking Menu **lub** Command Palette **lub** skrótu klawiszowego
- Preview w każdej operacji: **natychmiastowy** odczuwalnie
- Wyrażenia/parametry w każdym polu liczbowym: **tak**

### Techniczne
- Brak niszczenia historii przy direct edit
- Poprawne osadzanie nowych obiektów w aktywnym kontekście
- Wysoka odporność na zmiany topologiczne (TNP)
- Kompatybilność z istniejącymi dokumentami FreeCAD
