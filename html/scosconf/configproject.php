<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

// TODO: Create new projects!

$project = $_REQUEST["project"];
$name = $_REQUEST["name"];

if ($name && $name != "") {
    if ($_REQUEST["active"]) {
	$active = 1;
    } else {
	$active = 0;
    }

    // A name is given i.e. we shall create or update.
    if ($project && $project > 0) {
        // Update!
        mysql_query("UPDATE scos_project "
	    . "SET active='$active' "
	    . "WHERE id=$project");
    } else {
	mysql_query("INSERT INTO scos_project "
	    . "SET active='$active'");
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
