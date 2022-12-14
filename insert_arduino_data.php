<?php
ini_set('display_errors',1);
error_reporting(E_ALL);
include('config.php');

if(isset($_GET["uidString"])) {//this and others comes from Check_pointage.ino
    $uidString = $_GET["uidString"]; // get cardId value from HTTP GET
    $currentDate = $_GET["currentDate"]; // get date value from HTTP GET
    $heure = $_GET["heure"]; // get heure value from HTTP GET
    $etat = $_GET["etat"]; // get etat value from HTTP GET


/*
mysql> desc presences;
+--------------+-----------------+------+-----+-------------------+-----------------------------+
| Field        | Type            | Null | Key | Default           | Extra                       |
+--------------+-----------------+------+-----+-------------------+-----------------------------+
| id           | int(6) unsigned | NO   | PRI | NULL              | auto_increment              |
| uidString    | varchar(30)     | NO   |     | NULL              |                             |
| currentDate  | varchar(30)     | NO   |     | NULL              |                             |
| heure        | varchar(30)     | NO   |     | NULL              |                             |
| etat         | varchar(7)      | YES  |     | NULL              |                             |
| reading_time | timestamp       | NO   |     | CURRENT_TIMESTAMP | on update CURRENT_TIMESTAMP |
+--------------+-----------------+------+-----+-------------------+-----------------------------+
 */

   $sql = "INSERT INTO presences (uidString, currentDate, heure, etat) VALUES ($uidString, $currentDate, $heure, $etat)";
   if ($conn->query($sql) === TRUE) {
      echo "New record created successfully";
   } else {
    echo "insertion failed";
   }
}else {
    echo "Error: " . $sql . " => " . $conn->error;
}

?>