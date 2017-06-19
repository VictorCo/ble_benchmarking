@echo off
chcp 65001 > nul
SETLOCAL

rem VARIABLE===================================
SET byte=
SET debit=
SET time=
SET n_packet=
SET type_test=
SET interval_min=
SET interval_max=
SET timeout=
SET slave_latency=

SET test_down=DOWN
SET test_up=UP
SET empty=_

SET msg_start=Start timer
SET msg_stop=Stop timer
SET msg_octet=Octet
SET msg_debit=Debit
SET msg_time_up=UP get time
SET msg_time_down=DOWN get time
SET msg_n_packet=N packet
SET msg_interval_min=Interval min
SET msg_interval_max=Interval max
SET msg_slave_latency=Slave latency
SET msg_timeout=Timeout

SET msg_check=0
SET trouve=0

SET file_log=
SET file_excel=temps_transfert.csv

SET n_line=0

SET program=%~0 

rem ==============================================

echo ###############################
echo # Convertir .log vers excel   #
echo ###############################

setlocal enabledelayedexpansion


rem CHECK OPTION===================================================================

rem help
if "%~1"=="-?" goto help
if "%~1"=="/?" goto help

rem log sur disque dur
IF /I "%~1"=="-l" set custom_log=1
IF /I "%~1"=="/l" set custom_log=1
IF DEFINED custom_log (
	IF NOT "%~1"=="" (
		SET file_log="%~2"
		IF NOT EXIST "%~f2" (
			echo le fichier log "%~f2" specifié n'existe pas
			goto help
		)
		SHIFT
		SHIFT
	) ELSE GOTO help
)

rem excel perso
IF /I "%~1"=="-e" set custom_excel=1
IF /I "%~1"=="/e" set custom_excel=1
IF DEFINED custom_excel (
	IF NOT "%~1"=="" (
		IF "%~2" == "" (
			echo veuillez specifier un nom de fichier excel
			goto help
		)
		SET file_excel="%~2.csv"
		SHIFT
		SHIFT
	) ELSE GOTO help
)

rem adresse perso du smartphone
IF /I "%~1"=="-a" set custom_adress=1
IF /I "%~1"=="/a" set custom_adress=1
IF DEFINED custom_adress (
	IF NOT "%~1"=="" (
		IF "%~2" == "" (
			echo veuillez specifier une adresse valide
			goto help
		)
		SET device=%~2
		SHIFT
		SHIFT
	) ELSE GOTO help
)

IF NOT "%~1"=="" (
	echo %~1 : option inconnue
	GOTO help
)


rem ===============================================================================

rem Si on utilise un log custom pas besoin de le recuperer sur le smartphone
IF DEFINED custom_log goto run

rem VERIFIER SI ADB EST DANS LA VARIABLE PATH======================================
:adb
call adb devices > nul 2>&1
IF %ERRORLEVEL% GTR 0 (
	echo Erreur : ADB n'est pas reconnu
	echo          Ajouter [Android Bundle path]/sdk/platform-tools au %%PATH%%
	goto exit
)
rem ================================================================================

rem VERIFIER SI UN SMARTPHONE EST CONNECTE==========================================
:smartphone
if "%device%"=="" (
	rem regarde si il y a un device connecté
	for /f "delims=" %%a in ('call adb devices ^| findstr "device unauthorized emulator" ^| find /c /v "devices"') do (
		if "%%a"=="0" (
			echo Erreur: Aucun smarpthone connecté.
			goto exit
		)
		if not "%%a"=="1" (
			echo Erreur: Plus de 1 smarpthone connecté au PC 
			echo         Specifier le numéro de série du smartphone avec l'option d
			call adb devices
			goto exit
		)
	)
) else (
	rem regarde si le numero du smarpthone specifié est connecté
	for /f "delims=" %%a in ('call adb devices ^| find /c "%device%"') do (
		if "%%a"=="0" (
			echo Erreur: Le smarpthone avec le numero "%device%" n'est pas connecté.
			call adb devices
			goto exit
		)
	)
)
echo smartphone connecté : OK
rem ==============================================================================

rem si on utilise un fichier excel qui n'existe pas on le crée
IF NOT EXIST "%file_excel%" (
	echo Le fichier %file_excel% à été créé
	echo DeviceVersion;Android;Type test (UP ou DOWN^);Temps (ms^);octet;n paquet;Debit (Ko/s^);interval_min;interval_max;Slave_latency;Timeout > %file_excel%
)


rem OBTENIR LE DERNIER LOG DU SMARTPHONE==========================================
:last_log
adb shell ls /sdcard/Nordic\ Semiconductor | findstr /R "Log.[0-9]*-[0-9]*-[0-9]*.[0-9]*:[0-9]*:[0-9]*.txt" >> tmp
FOR /f "delims=" %%a in (tmp) do (
SET file_log=%%a
)
del tmp
IF "%file_log%" == "" (
	echo Aucun fichier log trouvé dans /sdcard/Nordic Semiconductor/
	goto help
)

echo %file_log%
call adb pull "/sdcard/Nordic Semiconductor/%file_log%" tmp
echo copie de %file_log%
SET file_log=tmp
rem Attente de 2 seconde
ping 192.0.2.3 -n 1 -w 2000 > nul
rem ==============================================================================


rem BOUCLE PRINCIPAL POUR EXTRAINE DONNEES===============================================
:run
echo Traitement en cours...
FOR /F delims^=^>^"^ tokens^=^1^,^2^,^3 %%A IN ('findstr /R "^A" "%file_log%"' ) DO (
	rem Stop timer
	echo %%B | find "%msg_stop%" > nul 2>&1
	if !errorlevel! == 0 (
		rem si on a des anciennes donnees a ajoute
		IF !n_line! GTR 0 call :write_excel "!type_test!" "!time!" "!byte!" "!n_packet!" "!debit!" "!interval_min!" "!interval_max!" "!slave_latency!" "!timeout!"
		
		rem on init les params
		SET byte=%empty%
		SET debit=%empty%
		SET time=%empty%
		SET n_packet=%empty%
		SET type_test=%empty%
		SET interval_min=%empty%
		SET interval_max=%empty%
		SET timeout=%empty%
		SET slave_latency=%empty%
		
		SET /A n_line+=1
		SET msg_check=1
		
	)
	echo %%B | find "%msg_start%" > nul 2>&1
	IF !errorlevel! == 0 (
		rem si on commence un nouveau timer on skip les donnees
		SET msg_check=0
	)
	
	IF !msg_check! == 1 (
		rem Octet
		IF "%%B" == "%msg_octet%" SET byte=%%C
		
		rem Debit
		IF "%%B" == "%msg_debit%" SET debit=%%C
		
		rem n_packet
		IF "%%B" == "%msg_n_packet%" SET n_packet=%%C

		rem interval min
		IF "%%B" == "%msg_interval_min%" SET interval_min=%%C

		rem interval max
		IF "%%B" == "%msg_interval_max%" SET interval_max=%%C

		rem slave latency
		IF "%%B" == "%msg_slave_latency%" SET slave_latency=%%C

		rem timeout
		IF "%%B" == "%msg_timeout%" SET timeout=%%C

		rem UP
		IF "%%B" == "%msg_time_up%" (
			SET time=%%C
			SET type_test=%test_up%
		)

		rem DOWN
		IF "%%B" == "%msg_time_down%" (
			SET time=%%C
			SET type_test=%test_down%
		)
	)

)

IF !n_line! GTR 0 call :write_excel "!type_test!" "!time!" "!byte!" "!n_packet!" "!debit!" "!interval_min!" "!interval_max!" "!slave_latency!" "!timeout!"

rem si on utilise pas un fichier perso de log on supprime le tempoarire
IF NOT DEFINED custom_log del tmp

goto exit
rem =====================================================================================

rem ECRITURE DONNEES DANS FICHIER EXCEL==================================================
:write_excel
SET add_line=%empty%;%empty%;%~1;%~2;%~3;%~4;%~5;%~6;%~7;%~8;%~9
echo %add_line%>>%file_excel% && echo Lignes (%add_line%) ajoutées dans (%file_excel%)
goto eof
rem =====================================================================================

rem HELP=================================================================================
:help
echo AIDE :
echo %program% [/L fichier_log] [/E fichier_excel] [/A adresse_smartphone]
echo    %program% permet de convertir un fichier .log obtenu après un test de benchmark vers un fichier excel
echo    Sans option le script ira chercher le log le plus récent sur le smartphone connecté en USB
echo    Le chemin par défaut du log est : /sdcard/Nordic Semiconductor/fichier_log
echo    Le chemin par défaut du fichier excel est : %file_excel%
echo    /L   Specifie un fichier log présent sur l'ordinateur
echo         ex : %program% /L custom_log
echo    /E   Specifie un fichier .csv pour enregistrer les données extraites
echo         Si le fichier .csv n'existe pas il sera créé avec les bonnes colonnes
echo         ex : %program% /E custom_transfert
echo    /A   Specifie un numéro de série de smarpthone sur lequel le fichier .log sera pris
goto exit
rem =====================================================================================

:exit
endlocal

:eof