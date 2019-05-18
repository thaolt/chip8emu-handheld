EESchema Schematic File Version 4
LIBS:chip8emu-board-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L chip8emu-board:STM32F407VET6 U?
U 1 1 5CE0492E
P 1300 1750
F 0 "U?" H 2600 2237 60  0000 C CNN
F 1 "STM32F407VET6" H 2600 2131 60  0000 C CNN
F 2 "LQFP-100" H 2600 2090 60  0001 C CNN
F 3 "" H 1300 1750 60  0000 C CNN
	1    1300 1750
	1    0    0    -1  
$EndComp
$Comp
L chip8emu-board:SN74LV125APW U?
U 1 1 5CDF0F89
P 6750 1450
F 0 "U?" H 7950 1837 60  0000 C CNN
F 1 "SN74LV125APW" H 7950 1731 60  0000 C CNN
F 2 "PW14" H 7950 1731 60  0001 C CNN
F 3 "" H 6750 1450 60  0000 C CNN
	1    6750 1450
	1    0    0    -1  
$EndComp
Text Label 5150 6900 0    50   ~ 0
LCD_VDD
Text Label 5150 7000 0    50   ~ 0
LCD_GND
Text Label 5150 7100 0    50   ~ 0
LCD_SCK
Text Label 5150 7200 0    50   ~ 0
LCD_SDA
Wire Wire Line
	5150 6900 5650 6900
Wire Wire Line
	5150 7000 5650 7000
Wire Wire Line
	5150 7100 5650 7100
Wire Wire Line
	5150 7200 5650 7200
$Comp
L chip8emu-board:LCD_Connector J?
U 1 1 5CE43481
P 5850 7000
F 0 "J?" H 5930 6992 50  0000 L CNN
F 1 "LCD_Connector" H 5930 6901 50  0000 L CNN
F 2 "" H 5850 7000 50  0001 C CNN
F 3 "~" H 5850 7000 50  0001 C CNN
	1    5850 7000
	1    0    0    -1  
$EndComp
$Comp
L chip8emu-board:Keypad_Connector J?
U 1 1 5CE43FED
P 9700 5500
F 0 "J?" H 9780 5492 50  0000 L CNN
F 1 "Keypad_Connector" H 9780 5401 50  0000 L CNN
F 2 "" H 9700 5500 50  0001 C CNN
F 3 "~" H 9700 5500 50  0001 C CNN
	1    9700 5500
	1    0    0    -1  
$EndComp
$Comp
L chip8emu-board:USB_B_Mini J?
U 1 1 5CE6CAF9
P 6900 5150
F 0 "J?" H 6957 5617 50  0000 C CNN
F 1 "USB_B_Mini" H 6957 5526 50  0000 C CNN
F 2 "" H 7050 5100 50  0001 C CNN
F 3 "~" H 7050 5100 50  0001 C CNN
	1    6900 5150
	1    0    0    -1  
$EndComp
$EndSCHEMATC
