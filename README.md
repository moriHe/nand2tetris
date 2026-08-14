Nand2Tetris --- Knowledge Document

Überblick

Dieses Repository dokumentiert praktische Arbeit am Nand2Tetris-Kurs und
damit den Aufbau eines Computersystems über mehrere Abstraktionsebenen
hinweg. Das Repository enthält die Projektverzeichnisse p1 bis p12
und zeigt insbesondere in den späteren Projekten selbst implementierte
Übersetzer-, Compiler- und Betriebssystembestandteile.

Technischer Schwerpunkt

Nand2Tetris verbindet Rechnerarchitektur, Maschinensprache, Assembler,
virtuelle Maschinen, Compilerbau und grundlegende
Betriebssystemfunktionen. Der besondere Wert des Projekts liegt darin,
dass nicht nur eine Anwendung auf vorhandenen Frameworks gebaut wird,
sondern die Abstraktionsschichten unterhalb normaler
Anwendungsentwicklung nachvollzogen und implementiert werden.

Implementierungen im Repository

Assembler

Im Umfeld von Projekt 6 ist eine Implementierung in C vorhanden. Das
Repository enthält erzeugte Hack-Maschinencode-Dateien wie Add.hack,
Max.hack, Pong.hack und Rect.hack. Damit wird die Übersetzung von
symbolischer Hack-Assemblersprache in binären Maschinencode behandelt.

VM Translator

Die Projekte 7 und 8 enthalten eine in C aufgebaute VM-Übersetzung. Zu
sehen sind unter anderem Parser-, Writer-, Stack-Pointer- und
Utility-Komponenten. Testfälle umfassen Stack-Arithmetik, Push/Pop,
Pointer/Static-Segmente, Schleifen, Funktionen und Fibonacci-Beispiele.
Die Aufgabe verbindet eine stackbasierte virtuelle Maschine mit der
darunterliegenden Hack-Assemblersprache.

Compiler Frontend

Projekt 10 enthält Arbeit an Tokenizer und Parser sowie erzeugte
XML-Repräsentationen für Jack-Programme wie Square und SquareGame. Damit
werden lexikalische Analyse und Syntaxanalyse einer höheren
Programmiersprache praktisch umgesetzt.

Compiler

Projekt 11 erweitert den Compiler um Symboltabellen und Codegenerierung.
Das Repository enthält eine symbol_table-Komponente und Ausgaben für
Programme wie Average, ComplexArrays, Pong und Square. Damit werden
Variablenauflösung, Scopes und die Übersetzung von Jack in die
VM-Zielsprache behandelt.

Jack OS / Laufzeitbibliothek

Projekt 12 enthält Implementierungen zentraler Jack-Bibliotheksklassen,
darunter Array, Keyboard, Memory, Output, Screen, String und
Sys, zusammen mit Tests. Dadurch wird nachvollziehbar, welche
Laufzeitdienste höhere Programme benötigen und wie diese auf einer sehr
kleinen Plattform bereitgestellt werden.

Demonstrierte Kenntnisse

C und systemnahe Programmierung

Rechnerarchitektur und Maschinensprache

Assemblierung und Codegenerierung

Stackmaschinen und virtuelle Maschinen

Parser und Tokenizer

Compilerbau

Symboltabellen und Scope-Verwaltung

Speicher- und Laufzeitkonzepte

Debugging über mehrere Abstraktionsebenen

Verständnis der Kette von Hochsprache bis Maschinencode

Einordnung

Das Projekt ist primär ein Computer-Science-/Lernprojekt und keine
klassische Produktanwendung. Für ein technisches Profil ist es besonders
relevant, weil es Grundlagen demonstriert, die bei normaler
Framework-Entwicklung häufig verborgen bleiben.

Gute Retrieval-Fragen

Welche Erfahrung gibt es mit Compilerbau?

Hat der Entwickler systemnah mit C gearbeitet?

Wie tief reicht das Verständnis von Rechnerarchitektur?

Wurde eine virtuelle Maschine oder ein VM-Translator implementiert?

Welche Erfahrung besteht mit Parsern, Tokenizern und Symboltabellen?

Wurden Betriebssystem- oder Laufzeitkomponenten implementiert?

Quelle

GitHub Repository: moriHe/nand2tetris
