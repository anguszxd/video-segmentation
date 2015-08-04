
#===============================
#  openUrlGRAY8 - entry point
#===============================
proc openUrlGRAY8 { } {
    set url [.topGray.entry get]
    exec ./vibe-gray.exe -i $url &
    destroy .topGray
} ;

#===============================
#  openUrlRGB24 - entry point
#===============================
proc openUrlRGB24 { } { 
    set url [.topColor.entry get]
    exec ./vibe-rgb.exe -i $url &
    destroy .topColor
} ;

#===============================
#  openStreamGRAY8 - entry point
#===============================
proc openStreamGRAY8 { } {
    toplevel .topGray
    wm title .topGray "Open Stream"
    label .topGray.label -text "Enter Network Stream URL here:" 
    entry .topGray.entry 
    .topGray.entry insert 0 "http://"
    button .topGray.button -text "Open Stream" -command "openUrlGRAY8"
    pack .topGray.label .topGray.entry -fill both
    pack .topGray.button 
} ;

#===============================
#  openStreamRGB24 - entry point
#===============================
proc openStreamRGB24 { } {
    toplevel .topColor
    wm title .topColor "Open Stream"
    label .topColor.label -text "Enter Network Stream URL here:"
    entry .topColor.entry
    .topColor.entry insert 0 "http://"
    button .topColor.button -text "Open Stream" -command "openUrlRGB24"
    pack .topColor.label .topColor.entry -fill both
    pack .topColor.button 
} ;

#=============================
#  openFileRGB24 - entry point
#=============================
proc openFileRGB24 { } {
    set myFile [tk_getOpenFile]
    exec ./vibe-rgb.exe -i $myFile &
} ;

#=============================
#  openFileGRAY8 - entry point
#=============================
proc openFileGRAY8 { } {
    set myFile [tk_getOpenFile]
    exec ./vibe-gray.exe -i $myFile &
} ;

#========================================
#  main - entry point
#========================================

#========================================
# explicitly setup our main window
#========================================
wm geometry  .   900x600+10+10
wm title  .   "ViBe"

#========================================
# setup the frame stuff
#========================================
destroy .myArea
set f [frame .myArea -borderwidth 5 -background gray]

#========================================
# add an image
#========================================
set myimage [image create photo -file ./input-background.gif]
label .imagedisplayer -image $myimage
pack .imagedisplayer -side top

#========================================
# add the text widget
#========================================
text $f.t -bg white -yscrollcommand ".srl_y set" -width 120
scrollbar .srl_y -command "$f.t yview" -orient v 
pack $f.t -side left -fill both
pack .srl_y -side right -fill both
pack $f -side bottom -expand true -fill both 

#==========================
# add a line of text
#==========================
$f.t insert end {
  Provided algorithm:
  -------------------
  This code implements the ViBe Background Subtraction algorithm described in:

  O. Barnich and M. Van Droogenbroeck. 
   ViBe: A universal background subtraction algorithm for video sequences.
   In IEEE Transactions on Image Processing, 20(6):1709-1724, June 2011. 
   Available online at: 
    http://ieeexplore.ieee.org/search/freesrchabstract.jsp?tp=&arnumber=5672785 
    http://orbi.ulg.ac.be/bitstream/2268/81248/2/Barnich2011ViBe.pdf

and 

  O. Barnich and M. Van Droogenbroeck. 
   ViBe: a powerful random technique to estimate the background 
   in video sequences. In International Conference on Acoustics, 
   Speech, and Signal Processing (ICASSP 2009), pages 945-948, April 2009. 

  Please use one of these entries to refer to our algorithm.

  Licence:
  --------
  ViBe is covered by a patent (see http://www.ulg.ac.be/telecom/research/vibe).
  We do not provide the source code but provide an object file and an interface.

  Copyright:
  ----------

  Tne University of Liege, Belgium, shall retain all property rights, title and 
  interest in the hardware or the software, documentation and trademarks.
 
  All the names of companies and products mentioned in the documentation may be the trademarks 
  of their respective owners.
 
  The licensing, use, leasing, loaning, translation, reproduction, copying or modification 
  of the hardware or the software, marks or documentation, is not allowed without prior notice.
 
  The University of Liege shall not be liable for any loss of or damage to revenues, profits, 
  goodwill, data, information systems or other special, incidental, indirect, consequential or 
  punitive damages of any kind arising in connection with the use of the hardware or the software of 
  the University of Liege or resulting of omissions or errors.

  Credits: 
  --------

  This package uses the following libraries: 
   - ffmpeg (LGPL license) [http://www.ffmpeg.org/]
   - SDL (LGPL license) [http://www.libsdl.org/]
   - TCL/TK TclKit (license free) [http://www.tcl.tk]


}

#========================================
# create a menubar
#========================================
destroy .menubar
menu .menubar
. config -menu .menubar

#========================================
#  create a pull down menu with a label 
#========================================
set File [menu .menubar.mFile]
.menubar add cascade -label File  -menu  .menubar.mFile

#========================================
# add the Open menu items
#========================================
$File add command -label "Open Video File for COLOR processing" -command openFileRGB24
$File add command -label "Open Video File for GRAYSCALE processing" -command openFileGRAY8
$File add command -label "Open Video Network Stream for COLOR processing" -command openStreamRGB24
$File add command -label "Open Video Network Stream for GRAYSCALE processing" -command openStreamGRAY8

#========================================
# add the menu item
#========================================
$File add command -label Quit -command exit

