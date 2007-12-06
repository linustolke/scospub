<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

$xml = $_GET['xml'];
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
    echo "<scos_projects>\n";
} else {
    page_head(tr(SCOSP_TITLE));
    echo tr(SCOSP_DESCRIPTION)."<br><br>
    ";
    start_table();
    echo "
        <tr>
          <th>".tr(SCOSC_PROJECTNAME)."</th>
          <th>".tr(SCOSP_THEPROJECT)."</th>
          <th>".tr(SCOSC_SOURCES)."</th>
        </tr>
    ";
}
$result = mysql_query("select * from scos_project");

while ($proj = mysql_fetch_object($result)) {
    $friendly_name = $proj->user_friendly_name;
    if (!$friendly_name) {
	$friendly_name = "?";
    }
    if ($xml) {
        echo "  <scos_project>\n";
        echo "    <id>$proj->id</id>\n";
	echo "    <active>$proj->active</active>\n";
        echo "    <name>$proj->name</name>\n";
    } else {
        echo "
            <tr>
              <td><a name='$proj->name'>$proj->name</a>
	";
	if ($proj->active) {
	    echo "(active)";
	} else {
	    echo "(not active)";
	}
	echo "
	      </td>
              <td>$friendly_name</td>
        ";

	echo "
	      <td>
                <ul>
	";
	$r2 = mysql_query('SELECT * '
		. 'FROM scos_source '
		. "WHERE project = $proj->id");

	while ($av = mysql_fetch_object($r2)) {
	    echo "<li>";
	    if ($proj->type == 1) {
		echo "<a href='subversionsrc.php?$av->id'>";
		echo tr(SCOSC_SVN_SOURCE_URL);
		echo " $av->url</a>";
            } else {
		echo "".tr(SCOSC_UNKNOWN_TYPE)."";
            }
	    echo "</li>";
	} else {
	    echo "
	        <td>".tr(SCOSC_NO_SOURCE)."</td>
	    ";
	}
        echo "
              </td>
            </tr>
        ";
    }

    if ($xml) {
        echo "  </scos_project>\n";
    } else {
        echo "
            </tr>
        ";
    }
}
mysql_free_result($result);

if ($xml) {
    echo "</scos_projects>\n";
} else {
    end_table();
    page_tail();
}
?>
