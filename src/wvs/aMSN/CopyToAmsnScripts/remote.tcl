##################################################################
### remote.tcl v 1.0	22'nd of May 2003    KaKaRoTo
###						15'th of April 2008  Michael McElligott
##################################################################

::Version::setSubversionId {$Id: remote.tcl 8548 2007-05-06 02:09:32Z kakaroto $}

if { $initialize_amsn == 1 } {
	global remote_port remote_auth remote_sock_lock remote_sock
   
	set remote_port 0
	set remote_auth 0
	set remote_sock_lock 0
	set remote_sock 0
	set remote_authtimer 0
}

proc remote_check_online { } {
	if { [::MSN::myStatusIs] != "FLN" } {
		write_remote "1| connected..."
		return
	} else {
		after 1000 "remote_check_online"
	}
}

proc remote_touchauthtimer {} {
	global remote_authtimer
	set remote_authtimer 0
}

namespace eval ::remote {

	proc checkmail { } {
		set unread [::hotmail::unreadMessages]
		write_remote "28| $unread" mailmsg1
	}

	proc getmailalertmsg { } {
		set unread [::hotmail::unreadMessages]
		set froms [::hotmail::getFroms]

		if {$unread == 0} {
			set mailmsg "[trans nonewmail]"
		} elseif {$unread == 1} {
			set mailmsg "[trans onenewmail]"
		} elseif {$unread == 2} {
			set mailmsg "[trans twonewmail 2]"
		} else {
			set mailmsg "[trans newmail $unread]"
		}
		write_remote "29| $mailmsg" mailmsg2
	}
	
	proc getmaildetails { } {
		set froms [::hotmail::getFroms]
		set fromsTextRemote ""
		
		foreach {from frommail} $froms {
			append fromsTextRemote " | $from @ $frommail"
		}
		write_remote "30|$fromsTextRemote" mailmsg3
	}
	
	proc ping { } { 
		write_remote "18| pong"
	}


	# connect 
	# connects you to your account
	#
	proc connect { } {

		if { [catch { ::MSN::connect } res] } {
			write_remote "2| connect error"
		} else {
			write_remote "3| connecting ..." 
			after 1000 "remote_check_online"
		}
	}

	# logout
	# logs you out from the current session
	#
	proc logout { } {
		write_remote "4| logout"
		::MSN::logout
	}

	# help
	# prints the help message from the remote.help file
	#
	proc help { } {

		set fd [open "remote.help" r]
		set printhelp [read $fd]
		close $fd

		write_remote "5| $printhelp"
  
	}

	# online
	# Shows list of users connected
	#
	proc online { } {
		foreach username [::MSN::getList FL] {
			set state_code [::abook::getVolatileData $username state]

			if { $state_code !="FLN" } {
				write_remote "6| $username - [::abook::getNick $username] ---:| [trans [::MSN::stateToDescription $state_code]]"
			}
		}
	}

	proc status { } {
		set nick [::abook::getPersonal MFN]
		write_remote "20| $nick"
		if {[ ::config::getKey protocol] == 11 } {
			set psm [::abook::getPersonal PSM]
			write_remote "21| $psm"
		}
		getstate
	}

	proc getstate { } {

		set my_state [::MSN::stateToDescription [::MSN::myStatusIs]]

		write_remote "7| $my_state"

	}

	proc setstate { state } {

		set state [string tolower $state]
		if { "$state" == "online" } {
			ChCustomState NLN
		} elseif { "$state" == "away" } {
			ChCustomState AWY
		} elseif { "$state" == "busy" } {
			ChCustomState BSY
		} elseif { "$state" == "noactivity" } {
			ChCustomState IDL
		} elseif { "$state" == "brb" } {
			ChCustomState BRB
		} elseif { "$state" == "onphone" } {
			ChCustomState PHN
		} elseif { "$state" == "lunch" } {
			ChCustomState LUN
		} elseif { "$state" == "appearoffline" } {
			ChCustomState HDN
		} else {
			write_remote "8| Invalid state" error
			return
		}
		write_remote "9| State changed"
	}

	proc listcustomstates { } { 
		set numstates [StateList size] 
		
		if {$numstates > 0} { 
			write_remote "22| ID State name" 
			
			for { set stateid 0 } { $stateid < $numstates } { incr stateid } { 
				set state [StateList get $stateid] 
				write_remote "23| $stateid\t [lindex $state 0]" 
			} 
		} else { 
			write_remote "24| No custom states defined" 
		} 
	}

	proc setcustomstate { state } { 
		set numstates [StateList size] 
		
		if { [string is digit $state] != 1 || $state < 0 || $state >= $numstates } { 
			write_remote "25| Invalid state" 
		} else { 
			ChCustomState $state 
		} 
	} 

    proc setpsm { args } { 
	    set psm [string map { \\\; \; \\\" \" \\\[ \[ \\\] \] \\\: \: \\\\ \\ \\\* \* \\\? \?} [join $args]]

	    ::MSN::changePSM "$psm" 
	    write_remote "26| $psm" 
    } 
 	
	proc setnick { args } {
	       set nickname [string map { \\\; \; \\\" \" \\\[ \[ \\\] \] \\\: \: \\\\ \\ \\\* \* \\\? \?} [join $args]]
		if {$nickname != ""} {
			::MSN::changeName [::config::getKey login] "$nickname"
			write_remote "10| New nick set to : $nickname"
		} else {
			write_remote "11| New nick not entered"
		}
	}

	proc amsn_close { } {
		# close_cleanup
		exit
	}

	proc whois { user } {

		set found 0

		foreach username [::MSN::getList FL] {
			if { "[::abook::getNick $username]" == "$user" } {
				write_remote "12: $user is : $username" 
				set found 1
				break
			}
		}
		if { $found == 0 } {
			write_remote "13: $user was not found in your contact list" error
		}
	}

	proc getuserstatus { username } {
		set state_code [::abook::getVolatileData $username state]
		write_remote "19| $username - [trans [::MSN::stateToDescription $state_code]]"
	}
	
	#Forward list		FL
	#Reverse List		RL
	#Accept List		AL
	#Block list			BL
	#Pending list 		PL

	proc getFL { } {
		 set clist [::MSN::getList FL]
		 write_remote "31| $clist" contactlist
	}

	proc getRL { } {
		 set clist [::MSN::getList RL]
		 write_remote "32| $clist" contactlist
	}

	proc getAL { } {
		 set clist [::MSN::getList AL]
		 write_remote "33| $clist" contactlist
	}
	
	proc getBL { } {
		 set clist [::MSN::getList BL]
		 write_remote "34| $clist" contactlist
	}
	
	proc getPL { } {
		 set clist [::MSN::getList PL]
		 write_remote "35| $clist" contactlist
	}

	proc getallcontacts { } {
		 set clist [::abook::getAllContacts ]
		 write_remote "36| $clist" contactlist
	}
					
	proc whatis { user } {

		set found 0

		if { [string match "*@*" $user] == 0 } {
			set user [split $user "@"]
			set user "[lindex $user 0]@hotmail.com"
			set user [string tolower $user]
		}	

		foreach username [::MSN::getList FL] {
			if { "$username" == "$user" } {
				write_remote "14| $user is known as : [::abook::getNick $user]" 
				set found 1
				break
			}
		}
		if { $found == 0 } {
			write_remote "15| $user was not found in your contact list..." error
		}
	}

	# msg { args }
	# sends a message to a user
	#
	proc msg { args } {
		global userchatto

		if { [info exists userchatto] } {
			set user "$userchatto"
			set message [string map { \\\; \; \\\" \" \\\[ \[ \\\] \] \\\: \: \\\\ \\ \\\* \* \\\? \?} [join $args]]
		} else {
			# This is to skip all the spaces that could be put before the user, like for example "msg       user1  my msg".
			# with the [split] we would get {msg {} {} {} {} {} {} user1 my msg}
			set idx 0
			set user {}
			while { [string length $user] <= 0 } {
				set user [lindex $args $idx]
				incr idx
			}
			set message [string map { \\\; \; \\\" \" \\\[ \[ \\\] \] \\\: \: \\\\ \\ \\\* \* \\\? \?} [join [lrange $args $idx end]]]
		}

		set message [string map { \{ "" \} ""} $message]

		if { [string match "*@*" $user] == 0 } {
			set user [split $user "@"]
			set user "[lindex $user 0]@hotmail.com"
		}

		set lowuser [string tolower $user]
   
		set win_name [::ChatWindow::For $lowuser]

		if { $win_name == 0 } {
			::amsn::chatUser "$user"

			while { [set win_name [::ChatWindow::For $lowuser]] == 0 } { }
		}

		#set input "${win_name}.f.bottom.in.input"
		set input [text ${win_name}.tmp]
		$input insert end "${message}"
	
		::amsn::MessageSend $win_name $input 
	
		destroy $input
	
	}
	
	proc chatto { user } { 
		global userchatto
	
		if { [string match "*@*" $user] == 0 } {
			set user [split $user "@"]
			set user "[lindex $user 0]@hotmail.com"
		}

		set userchatto "$user"

	}

	proc endchat { } {
		global userchatto
		if { [info exists userchatto] } {
			unset userchatto
		}
	}

}

proc write_remote { dataout {colour "normal"} } {
	global remote_sock

	set dataout [string map [list "\n" " $colour\n"]  $dataout]
  
	catch {puts $remote_sock "$dataout $colour"}
}

proc read_remote { command sock } {
	global remote_auth remote_sock

	if { "$remote_sock" != "$sock" } {
		set remote_temp_sock $remote_sock
		init_remote $sock
		if {  $remote_auth == 1 } {
			write_remote "16| Remote controlling is already active"
			init_remote $remote_temp_sock
			return 0
		}
	}


	if {$command != ""} {
		if { $remote_auth == 0 } {
			authenticate "$command" "$sock"
		} elseif { [catch {eval "::remote::$command" } res] } {
			write_remote "17| syntax error : $res" error
		}
	}
}


proc md5keygen { } { 
	set key [expr rand()]
	set key [expr $key * 1000000]

	return "$key"
}

proc authenticate { command sock } {
	global remotemd5key remote_auth remote_sock_lock

	if { $command == "auth" } {
		set remotemd5key "[md5keygen]"
		write_remote "auth $remotemd5key"
		# write_remote "[::md5::hmac $remotemd5key [list [::config::getKey remotepassword]]]"
	} elseif { [lindex $command 0] == "auth2" && [info exists remotemd5key] } {
		if { "[lindex $command 1]" ==  "[::md5::hmac $remotemd5key [list [::config::getKey remotepassword]]]" } {
			if { [::config::getKey enableremote] == 1 } { 
				set remote_auth 1
				set remote_sock_lock $sock
				write_remote "Authentication successfull"
			} else {
				write_remote "User disabled remote control"
			} 
		} else {
			if { [::config::getKey enableremote] == 1 } { 
				write_remote "Authentication failed"
			} else { 
				write_remote "User disabled remote control"
			}	
		}	
		unset remotemd5key
	} else {
		write_remote "syntax error : $command" error
	}
}


proc init_remote { sock } {
	global remote_sock

	set remote_sock $sock
}

proc close_remote { sock } {
	global remote_sock_lock remote_auth

	#write_remote "27| shuting down"
	if { $remote_sock_lock == $sock } {
		set remote_auth 0
	} 
}

proc init_remote_DS { } {
	catch {socket -server new_remote_DS 63251}
}

proc new_remote_DS { sock addr port } {

	fileevent $sock readable "remote_DS_Hdl $sock"
	fconfigure $sock -buffering line
}

proc remote_DS_Hdl { sock } {

	set email [gets $sock]
	if {[eof $sock]} {
		catch {close $sock}
	} else {
		grep $email $sock
	}
}

proc grep { pattern sock } {
	global HOME2

	set filename "[file join $HOME2 profiles]"

	if { [string match "*@*" $pattern] == 0 } {
		set pattern [split $pattern "@"]
		set pattern "[lindex $pattern 0]@hotmail.com"
	}

	if {([file readable "$filename"] != 0) && ([file isfile "$filename"] != 0)} {
	
		set file_id [open "$filename" r]
		gets $file_id tmp_data
		if {$tmp_data != "amsn_profiles_version 1"} {	;# config version not supported!
			puts $sock "versionerror"
			close $file_id
			return 0
	   	}

		# Now add profiles from file to list
		while {[gets $file_id tmp_data] != "-1"} {
			set temp_data [split $tmp_data]
			if { [lindex $temp_data 0] == "$pattern" }  {
				close $file_id
				puts $sock "[lindex $temp_data 1]"
				return 1
			}
		}
		puts $sock "invalid"
		close $file_id
		return 0
	} else {
		puts $sock "noexist"
		return 0
	}
}
