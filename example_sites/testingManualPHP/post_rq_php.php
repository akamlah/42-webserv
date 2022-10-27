<?php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
  // collect value of input field
  // $_POST['fname'] = 'FUCKYU';
  $name = $_GET['fname'];
  echo $_SERVER['QUERY_STRING'];
  echo "\npost:\n";
  echo $_POST["fname"];
  echo "\nonly post:\n";
  echo $_POST;
  echo "\n server requestmethod\n";
  echo $_SERVER["REQUEST_METHOD"];
  echo "\n";
  if (empty($name)) {
    echo "Name is empty";
} else {
	echo $name;
}
}
else
	echo "Not POST";

?>