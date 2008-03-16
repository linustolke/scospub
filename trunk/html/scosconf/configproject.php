<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

$project = $_REQUEST["project"];
$name = $_REQUEST["name"];
$team = $_REQUEST["team"];

if ($name && $name != "" && $team && $team != "" && $team != 0) {
    if ($_REQUEST["active"]) {
	$active = 1;
    } else {
	$active = 0;
    }

    // A name is given i.e. we shall create or update.
    if ($project && $project > 0) {
        // Update!

        // We cannot change the team!

        mysql_query("UPDATE scos_project "
	    . "SET name='$name', active='$active' "
	    . "WHERE id=$project");
    } else {
	mysql_query("INSERT INTO scos_project "
	    . "SET name='$name', team='$team', active='$active'");
    }

    header("Location: projects.php");
    exit;
}

page_head(tr(SCOSC_TITLE));

$name='';
$userfn='';
$active=1;
$team=0;

if ($project && $project > 0) {
    $result = mysql_query("SELECT * FROM scos_project WHERE id=$project");

    if ($line = mysql_fetch_object($result)) {
	$name = $line->name;
	$active = $line->active;
	$team = $line->team;
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
      <TD>Team:</TD>
      <TD>
        <SELECT name='team'>";
$result = mysql_query("SELECT id, name FROM team");
while ($line = mysql_fetch_object($result)) {
    echo "<OPTION value='$line->id'";
    if ($line->id == $team) {
	echo " selected='selected'";
    }
    echo ">$line->name</OPTION>";
}
mysql_free_result($result);
echo "
        </SELECT>
    </TR>
    <TR>
      <TD>Name:</TD>
      <TD><INPUT type='text' name='name' ";
echo "value='$name' ";
echo "/>
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
