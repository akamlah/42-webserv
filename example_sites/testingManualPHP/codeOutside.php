<?php
	$_POST['fname'] = "Bob";
	echo $_SERVER['PHP_SELF'];
?>
<?php
// if ($_SERVER["REQUEST_METHOD"] == "POST") 
// {
// }
    // collect value of input field
    $name = $_POST['fname'];
    if (empty($name)) {
        echo "Name is empty";
    } else {
        echo $name;
    }
?>