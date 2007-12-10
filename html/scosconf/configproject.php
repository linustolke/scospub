<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

$project = $_REQUEST["project"];
$name = $_REQUEST["name"];

if ($name && $name != "") {
    $userfn = $_REQUEST["user_friendly_name"];
    if ($_REQUEST["active"]) {
	$active = 1;
    } else {
	$active = 0;
    }

    // A name is given i.e. we shall create or update.
    if ($project && $project > 0) {
        // Update!
        mysql_query("UPDATE scos_project "
	    . "SET name='$name', user_friendly_name='$userfn', active='$active' "
	    . "WHERE id=$project");
    } else {
	mysql_query("INSERT INTO scos_project "
	    . "SET name='$name', user_friendly_name='$userfn', active='$active'");
    }

    header("Location: projects.php");
    exit;
}

page_head(tr(SCOSC_TITLE));

$name='';
$userfn='';
$active=1;

if ($project && $project > 0) {
    $result = mysql_query("SELECT * FROM scos_project WHERE id=$project");

    if ($line = mysql_fetch_object($result)) {
	$name = $line->name;
	$userfn = $line->user_friendly_name;
	$active = $line->active;
    }
    mysql_free_result($result);
}

echo "
<FORM METHOD='get' ACTION='configproject.php'>
";

if ($project && $project > 0) {
    echo "  <INPUT type='hidden' name='project' value='$project'/>
    ";
}

echo "
  <TABLE>
    <TR>
      <TD>Name:</TD>
      <TD><INPUT type='text' name='name' value='$name'/></TD>
    </TR>
    <TR>
      <TD>User Friendly Name:</TD>
      <TD><INPUT type='text' name='user_friendly_name' value='$userfn'/></TD>
    </TR>
    <TR>
      <TD>Active:</TD>
      <TD><INPUT type='checkbox' name='active' ";
if ($active != 0) {
    echo 'checked';
}
echo "/>
    </TR>
  </TABLE>
  <INPUT type='submit' value='Submit'/>
</FORM>
";

page_tail();
?>
