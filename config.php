<?php
// Informations d'identification
    $servername = "votre_serveur"; //exemple : $servername = "localhost";
    $username = "nom_utilisateur_sql"; //exemple : $username = "root";
    $password = "mot_de_passe_utilisateur_sql"; //exemple : $password = "passer";
    $dbname = "votre_base_de_donnees"; //exemple : $dbname = "pointage_db";
   // Create connection
   $conn = new mysqli($servername, $username, $password, $dbname);
 
// Vérifier la connexion
  if($conn === false){
    die("ERREUR : Impossible de se connecter. " . mysqli_connect_error());
  }
?>
