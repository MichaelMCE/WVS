

WVS v0.30.15


Media players supported:
Winamp 5.30 or later
MediaMonkey 3
AIMP 2
XMPlay 3.4.2.18 or later





Before installing:
As other G15 media plug-ins are known to conflict with WVS they should be disabled.
Exit the media player before running the installer.


Winamp:
Download wvs_03011-winamp-setup.exe then install to 'Winamp/Plugins' directory.
In Winamp's Preferences -> Plug-ins; put a tick beside "Auto execute visualization plug-in on playback".
Also in Preferences -> Plug-ins -> Visualisations; select "vis_wamp.dll".
On restarting Winamp for the first time, WVS may throw a pop-up indicating that it can not find a .dll.
This is normal and expected, if, and as long as the above instructions have been followed the pop-up will not reappear.

AIMP2:
Download wvs_03011-aimp-setup.exe then install to 'Aimp2/Plugins' directory.
In AIMP ensure vis_wvs.svp is activated by placing it in the right hand column of the Plugins configuration screen.
Deselecting either vis_wvs.svp or gen_wamp.dll will crash AIMP and WVS, so don't do it!

MediaMonkey:
Download wvs_03011-mediamonkey-setup.exe then install to the 'MediaMonkey/Plugins' directory.
With MediaMonkey the visualization can be manually initiated by clicking the 'Vis' icon.

XMPlay:
Download wvs_03011-xmplay-setup.exe then install in to the XMPlay directory.

For XMPlay, WVS must be manualy started via the DSP dialog. This may change in a future update.
Enter XMPlay's Options screen.
Click DSP
click on the 'Plugins' drop down menu.
Click 'WVS client. ...'
Click Add. Do not click 'Remove' or bad things will happen.
These  steps must be performed each time XMPlay is loaded.
Search the XMPlay forum for the latest beta version of xmplay.exe


As of 0.30.15 WVS requires lglcd.dll. This file should be copied to the location of the media player executable.
Eg; For winamp which is installed in to c:\program files\winamp\, lglcd.dll should be copied to c:\program files\winamp\


Operating WVS:

 Global keys: These keys are functional from any page.
Soft key 1: Go to previous page (applet).
Soft key 2: Go to next page (applet).
Holding Left-Shift + Wheel adjusts volume.
Control + Soft key 2: Toggle Dancinglights on and off.
Left Alt + Soft key 1: Go directly to default page.
Soft keys 1 and 4: Exit WVS.


By default the following pages are disabled: Mystify, Ping, RSS, ATI Tray Tools, µTorrent.
Use the config page to enable. Rememeber to save this or any other change.


 Media player (also known as 'Title') page: Displays audio graphs, track title, general track info, etc..
Soft key 3: Switch between the various audio graph displays modes.
 Mode:
 	(1) Spectrum - 1 pixel width columns.
	(2) Spectrum - 4 pixel width columns with 1 pixel space.
	(3) Oscilloscope - 1 pixel deep.
	(4) Oscilloscope - 2 pixel deep, easier to see than 3.
	(5) Oscilloscope - 4 pixel point blocks.
	(5) Two channel VU - top is left channel, bottom is right channel.
	(6) Off.

Press Soft key 4 once to enter track seek mode, denoted by the bottom right '<>' icon.
Use volume wheel to skip through track. Skip time is set in the config page.

Press Soft key 4 a second time to enter the Dancinglights (blinking lights) bandwidth calibration mode,
as denoted by the two vertical bars and the box with two numbers. This should only be used in conjunction with graph display 1 (1 pixel spectrum).
Each bar represents a frequency cut-off point which as a whole forms a band pass filter.
Left bar represents the lower frequency cut-off with the actual frequency displayed on the left, right is upper cutoff point and its frequency is displayed to the right.
In other words, this mode sets how the lights flash in response to the audio. Much like an Equaliser, adjust to your taste.
Use the wheel to move the band, hold left and right control to adjust lower and upper band respectively.
The settings are saved in the config as 'Lower BW limit' and 'Upper BW limit'.

Hold Right-Shift (RShift) to enter Hitlist config mode. Hitlist is an internal and user configurable playlist.
This will create an overlay on top of the graph which will display 'Hate it 1 Like it'.
The internal Hitlist is actually a UTF8 encoded file named 'hitlistX(X = 1 to 9).m3u8' which is saved in the wvs.exe directory.
Nine hitlists are available to use. Currently selected Hitlist is denoted by the centre digit within the overlay.
eg, 'Hate it 3 Like it' refers to hitlist number 3 with the filename 'hitlist3.m3u8'.
Use wheel to select hitlist.
Soft key 3 ('Hate it') searches and removes currently playing track from list.
Soft key 4 ('Like it') adds currently playing track (its complete path) to end of list. Duplicates are prevented.
There is a separate page dedicated to displaying and playing the contents of the Hitlist.
Unless you're familar with UTF8 encoding it is important that you do not manually edit this file.

Control + Soft Key 1 selects which of the three volume controls the wheel should adjust.
Options are: Master, Wave out and Media player volume.

Alt + Softkey 4 can be used to switch between Big5 and GBK encoding.



 Meta tag (Id3v2) page: Displays ID3v2 tags from tracks on the playlist.
WVS requests and displays the following tags sourced from the media player: 
"Title, Artist, Album, Track, Year, Genre, Comment, Length, Bitrate, 
SRate, Stereo, VBR, BPM, Gain, AlbumGain, Encoder, AlbumArtist, 
OriginalArtist, Disc, Remixer, Media, Lyricist, Subtitle, Mood, 
Key, Composer, Conductor, Publisher, Copyright and URL"
If available the complete path of the track will be displayed last.
Tag is displayed on the left followed by a colon ':' then the returned meta string, if any.
Fonts can be selected via Control + Softkey 3.
Available fonts are: 5x7, 7x6, 8x8, 8x9, 10x12, 14x16 and 18x18. Both 14x16 and 18x18 contain Chinese, Japanese and Korean (CJK) glyphs.

Use volume to navigate through the tags, hold Right-Shift to cycle through the playlist.
Current track can be played by pressing Soft key 3, which also resets the playlist position.



 Playlist: Displays the Media players playlist.
Use wheel to cycle through list.
Hold Right-Shift while using wheel to skip 'n' percent tracks. Percentage value is set in the config screen.
Currently playing track is highlighted between '>>' and '<<'.
Softkey 3 toggles play state (Pause), but only if "Handle Media Keys" is enabled.
Softkey 4 toggles track number on and off.
Control + Softkey 4: Flush string cache.
To play the selected track use Soft key 3.
Alt + Softkey 4 can now be used to switch between Big5 and GBK encoding.
Font can be selected via Control + Softkey 3.
Available fonts are: 5x7, 7x6, 8x8, 8x9, 10x12, 14x16 and 18x18.
14x16 and 18x18 are the only two that contain Asian glyphs.



 Hitlist: Displays contents of the selected Hitlist file.
Use wheel to cycle through list.
Hold Right-Shift while using wheel to skip 'n' percent tracks
Use Alt+Soft key 3 to removed highlighted track from hitlist file.
Use Shift+Soft key 3 to add highlighted entry to the media players playlist.
To play highlighted track use Soft key 3.
Soft key 4 toggles track number display.
Font can be selected via Control + Softkey 3.
Available fonts are: 5x7, 7x6, 8x8, 8x9, 10x12, 14x16 and 18x18.



 Equalizer: Displays the media players internal EQ states.
Currently only available under Winamp.
Soft key 3 and 4 or Wheel+Right-Shift: Select column to adjust.
Wheel adjust selected column.



 Lyrics: Displays song lyrics via MiniLyrics sourced .lrc data files.
Control + Soft key 4: Reload/search for lyric file.
Control + Wheel: Adjust per track time stamp offset by 250ms.
Soft key 4: Reset above stamp adjustment.
Soft key 3: Switch between tag and lyric screen.

As with the Fraps overlay, the Lyrics page must be enabled to enable the lyrics overlay on to the main page.
MiniLyrics does not need to be running for the Lyrics page to operate but it must be installed.
It is safe to remove gen_MiniLyrics.dll.

Setup:
Open wvs.cfg then add the below line containing the path of your lyrics folder:
lyrics_location=c:\lyrics ; this is a UTF8 formatted path



 aMSN: A plug-in for the aMSN MSN client with message sending ability.
Install:
Install aMSN from http://www.amsn-project.net (tested with 0.95 and 0.97rc1).
Close aMSN if running.
Copy remote.tcl from the WVS package to the amsn/scripts/ directory, Click 'Yes' when asked to confirm file replace (over right).
Start and configure aMSN with your MSN account details.
Once configured go to Account -> Preferences -> Connection -> Remote Connection Preferences. Tick 'Enable Remote controlling' then provide a password.
Click Save.
Open wvs.cfg with notepad.
Add the above remote control password to 'amsn_remotepassword='
Add your account (email) address to 'amsn_emailaddress='
Restart WVS.
Enable then go to the aMSN page, press Control + Soft key 3 to connect to aMSN which then connects to the MSN network, if not already.

If all goes well you should now be looking at your on-line contacts from top to bottom.
Use the wheel to scroll the contact list.
Press Soft key 4 to switch between detailed and standard display mode.
In standard mode only the user name is displayed, which helps to reduce the clutter.

Description of per contact detailed mode is as such "Internal ID #: Status: User name: email address"
eg, "3: Idle: Michael: someone@hotmail.co.uk"

Soft key 3 is used to cycle through the various available contact lists and your Inbox.
List order:
	(1) On-line contact (default) list where all user chat is also displayed, including your own.
	(2) Same as above but in an 'IRC like' style.
	(3) Contact list including off-line users. These are the only users you may send messages to.
	(4) Reverse list. People who have added you to their list.
	(5) Allowed list. A list of those who are permitted to contact (MSN) you.
	(6) Blocked list. People you have blocked from contacting you.
	(7) Pending list. Those awaiting your 'add to contact list' confirmation.
	(8) All contacts. All of the above in a single list.
	(9) Inbox. Displays total number of unread mails along with the sender name and address of any mails received during the current session.

Applet also contains an individual display page per on-line user. This is accessible by holding Right-Shift + volume to cycle through each.
Messages can be sent to any on-line user via either of two methods:
	(1) In the on-line contact list screen open an edit box (IME - see below), enter the users number (from detailed mode) followed by a colon ':', type your message then hit Enter.
	    eg, I want to send a message to "Sean" so I press Soft key 4 to find out Sean's ID number, which is 3.
	    I enter IME mode (see below) then type "3: Hello Sean!" then I press enter.
	    The space between ':' and 'Hello' isn't required.
	    Sean's ID number is 3 which will remain constant throughout the current WVS connection to aMSN.
	    ie, ID references will only change if WVS disconnects from aMSN.
	(2) Go to the individual page of that user (RShift + wheel), open edit box, type message then press enter.

It is normal for the edit box to close after pressing Enter.
Any message that could not be sent remains in the edit box buffer.
Press 'Control + /' (forward slash) to open the edit box, which is also known as an Input Method Editor (IME).
IME may also be activated by pressing 'Control + Enter', 'Control + Backspace', 'Right Control + Right Shift' and 'Control + Soft-key 4'.
IME activation key is selected in the config page and default is 'Ctrl+/'.
IME buffer is limited to 512 characters.



 µTorrent: µTorrent stats via the web interface.
Soft key 3: Switch between list view, detailed view and general overview.
Soft key 4: Cycle list view sort mode. Sort modes: Status, Completed, Name, Download rate, Upload rate, ETA and by Label.
Control + Soft key 3: Change font.
Contorl + Soft key 4: Toggle list view column alignment.
Right shift: Display current sort mode.

µTorrent setup:
In µTorrent's preferences find then enable the Web interface.
Add a username and password then below specify a port.
Open wvs.cfg with notepad, find then modify the 'µtorrent_nnnnnn' fields with the above information.
Do not enable µTorrent's guest account.



 Bandwidth: Displays real-time network bandwidth usage, with a scrolling clock.
Clock display is toggled on and off via soft-key 4.
Press soft key 3 to cycle through each detected network interface, which also re-numerates on each press.
Two display mode are available; Overlapped and Split. Overlapped is the default mode.
	(1) Overlapped: In overlapped mode total downloaded megabytes for the selected interface is displayed on the top left corner, total uploaded megabytes can be found in the top right corner.
		Current download rate through the interface card is displayed on the lower left, in kilobytes per second, likewise total uploaded kilobytes is found in the far right.
		Graph scale (peak transfer rate) is the top middle number, in kilobytes.
	(2) Split: Screen is split vertically with incoming data on the left and outgoing on the right.
		Three values are displayed per dataset: Peak transfer rate, which is also used to scale the graph, total transferred (per direction) through the interface and current transfer rate.
		Incoming: Peak rate is displayed on top left with the total to its right. Current rate can be found on the bottom left.
		Outgoing: Peak rate is displayed on top right with the total to its left. Current rate is displayed on the lower right.
		

 
 Netstat: Displays your external IP and various other statistics similar to Window's 'netstat' console application.
This applet was created with the sole purpose of displaying open ports, connections and processes with open ports.
This allowed (me) to quickly determine which ports required forwarding for a particular application/game.
As such it displays as much information as is available without duplicating too much from the Process List page.
There are three display modes available:
	(1) A configurable connection (and/or port) per line in a scrollable top down view.
	    To configure this page use volume to cycle through the options then Soft Key 4 to [de]activate.
	(2) A multiple page format with a single port per page. i.e., if there are 25 ports then there are 25 pages.
	(3) A per process top down view similar to 1.
	    If enabled first line will display your external IP.

Information available per port:
Protocol, Local IP:Port, Remote IP:Port (if TCP), Process ID, Module path (from PID) and Connection state.



 Process list: A task manager displaying various process statistics.
Process page is split in to three separate screens.
	{1} Process list. One process per line, displaying module name, memory used, total threads and process ID.
		Most recently created process is topmost in the list, under lies the 2'nd last created process and so on.
		Use Volume to scroll and to highlight a process.
		Soft key 4 toggles toggles heading display on/off.
	(2) Per process module list. Displays all modules within a process as well as the path of the process. 
		Process index, PID, Total threads and module name.
		Volume function is indicated by the arrow in the top right corner. 'Left' means volume cycles process list, 'Down' indicates volume cycles module list.
		Use volume to cycle through the process list (a left arrow is displayed), displaying the modules in each.
		Or press Soft key 4 to toggle module cycling (a down arrow is displayed).
	(3) Per process memory stats. Functions similar to module list.
		Displays Mem usage, Peak mem usage, Page faults, paged pool usage, peak paged pool usage, non paged pool usage, peak paged pool usage.
		
At any point a process may be terminated by pressing Control + Soft key 3.



 System info:
Displays free disk space of each drive, CPU usage, OS uptime and a few memory stats.



 Ati Tray Tools (v1.2.6):
Displays GPU and environment temp, FPS, AA & AF settings, GPU & memory frequency and fan speed.
For this to function OSD must be enabled within the AtiTT options, refer to Ati Tray Tools for details on this.



 RSS: Downloads and displays an RSS 2.0 feed.
Pause scrolling with Soft key 3.
Use soft key 4 to update feed.
Use volume to seek through stream. Right Shift + volume to fast forward/rewind.
Feed is automatically updated every 600 seconds.
Add feed URL to wvs.cfg.



 Ping: Displays a graph of ICMP echo's.
Add address to ping to wvs.cfg.
Use dot notation only. Ie, '127.0.0.1' is acceptable, 'localhost' is not.



 Fraps: Displays FPS and module name.
Fraps must be running before WVS/media player is started.
It is possible to overlay the FPS on to any other page. To do so, overlay must be enabled in the config page.



 Calendar: Displays date and time.
Use volume to select month.
Soft key 4 returns to current date.



 Config:
Config settings are grouped together in to pages. A page is opened and closed by pressing soft key 3 on its highlighted title.
Volume is used navigate through the various options and settings.
Soft key 3 switches between config option and its setting (left and right column). Use volume to adjust setting.
Soft key 4 applies all changes made.
Only settings within this page are saved.

To save your settings:
Navigate to the bottom of the config screen to the 'General' page then to the 'Profile' option.
Select the 'Save now' option.
Press Soft key 4 to apply this change.
Settings are now saved in profile.cfg.




Troubleshooting:
WVS employs a client-server architecture to communicate data between the client (wvs.exe/gen_wvs/xmp-wvsclient) and the media player.
As WVS employs a UDP socket (43675) for this communication, your Firewall and/or Anti-Virus program may perceive WVS as a threat.
Which, depending on your configuration, may terminate the WVS/media player process.
Reconfigure or disable either or both the Firewall and Anti-Virus program to prevent this from occurring.

Why wvs.exe, Sockets and why this complicated?
WVS after 0.30.4 was designed to operate independently of the media player and with the ability to remotely control that player through the keyboard.
What this allows you to do is to run Winamp(or whatever) on you home theatre set-up in one room whilst controlling the lot from the machine in the next room(or wherever).
I've tested this, it does work.

Getting unwanted "No disk in Drive A:" messages?
This is due to the Sysinfo page scanning free space of removable drives.
To disable set "List removable drives" to "No" in the 'System info' section of the Config page.
Save then restart.

To use WVS without the media player functionality: (Does not apply to the installers - wvs_xxxx.setup.exe)
In the Config page disable the following pages: Meta, Media player, Playlist, Hitlist and EQ.
In the General section set "Wait for player" to "No"
Save then restart.

To run WVS without waiting for the player: (default behaviour)
In the Config page -> General section set "Wait for player" to "No"
Save then restart.
WVS will auto detect player once its been started.

MS Windows Vista.
To fix the volume control issue set wvs.exe/media player in to 'Windows XP compatibility mode'.

WVS starts and then quickly closes or appears to crash.
Thats a symptom of an over-reactive Anti-Virus and/or Firewall protection scheme.
Add WVS to the "I'm a good guy" filter.



WVS, for your every LCD need:
http://mylcd.sourceforge.net

WVS support thread:
http://www.g15forums.com/forum/showthread.php?t=3916

Supported media players:
MediaMonkey	http://www.mediamonkey.com
XMPlay		http://www.un4seen.com
Winamp		http://www.winamp.com
AIMP		http://www.aimp.ru

NSIS installer used by WVS:
http://nsis.sourceforge.net/

WVS 0.30.4 in action:
http://www.youtube.com/watch?v=3ELEeO2wqk8

WVS on a 320x240 Sed1335 controlled LCD.
http://mylcd.sourceforge.net/wvs_images/

WVS on the G15:
http://mylcd.sourceforge.net/images/g15_wvs1.jpg
http://mylcd.sourceforge.net/images/g15_wvs2.jpg


WVS:
Written by Michael McElligott.

Credits and a thank you to..
NSIS installer by Alex Pytte.
Calendar code submitted by Hung Ki, updated and maintained by Michael McElligott.
Mystify code submitted by Hung Ki, updated and maintained by Michael McElligott.
Particles originally by Camilla Berglund (GLFW). Heavily modified and updated by Michael McElligott.
WVS testing, assistance, ideas, .ico icons, support, video and PDF manual by Chris "Interceptor One" Morley.



