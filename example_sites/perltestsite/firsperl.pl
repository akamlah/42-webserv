#!/usr/local/bin/perl
$userName =  $ENV{'LOGNAME'}; 
print "<!DOCTYPE html>
<html>
<head>
</head>
<body>
	<h1>This is a Perl script generated website.</h1>
	<h1>Made by,  $userName's machine</h1>
</body>
</html>";
exit (0);