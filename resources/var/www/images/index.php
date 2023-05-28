<!DOCTYPE html>
<html>
<head>
  <title>Personal Information Form</title>
</head>
<body>
  <h1>Personal Information Form</h1>
  <?php
  if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $name = $_POST["name"];
    $email = $_POST["email"];
    $dob = $_POST["dob"];

    echo "<h1>Congratulations, $name!</h1>";
    echo "<p>You have successfully submitted the form.</p>";
    echo "<p>Name: $name</p>";
    echo "<p>Email: $email</p>";
    echo "<p>Date of Birth: $dob</p>";
  } else {
      echo '<form action="index.php" method="post">';
      echo '<label for="name">Name:</label>';
      echo '<input type="text" id="name" name="name" required>';
      echo '<br>';
      echo '<label for="email">Email:</label>';
      echo '<input type="email" id="email" name="email" required>';
      echo '<br>';
      echo '<label for="dob">Date of Birth:</label>';
      echo '<input type="date" id="dob" name="dob" required>';
      echo '<br>';
      echo '<input type="submit" value="Submit">';
      echo '</form>';
  }
  ?>
</body>
</html>