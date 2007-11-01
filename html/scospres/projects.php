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
          <th>".tr(SCOSP_THEPROJECT)."</th>
          <th>".tr(SCOSP_NUM_REPORT)."</th>
          <th>".tr(SCOSP_LAST_REPORT)."</th>
        </tr>
    ";
}
$result = mysql_query("select * from scos_project where active=1");

while ($proj = mysql_fetch_object($result)) {
    if ($xml) {
        echo "  <scos_project>\n";
        echo "    <name>$proj->name</name>\n";
    } else {
        echo "
            <tr>
              <td>$proj->user_friendly_name</td>
        ";

	$r2 = mysql_query("select count(*) as num from scos_result where projectid=$proj->id");
	if ($av = mysql_fetch_object($r2)) {
	   echo "
		  <td>$av->num</td>
	   ";
	} else {
	   echo "
		  <td>".tr(SCOSP_NO_RESULTS)."</td>
	   ";
	}

	$r3 = mysql_query("select date from scos_result where projectid=$proj->id order by date desc limit 1");
	if ($av = mysql_fetch_object($r2)) {
	   echo "
		  <td>$av->date</td>
	   ";
	} else {
	   echo "
		  <td>".tr(SCOSP_NO_RESULTS)."</td>
	   ";
	}

        echo "
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
