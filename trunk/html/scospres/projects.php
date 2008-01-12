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
$result = mysql_query('SELECT scos_project.id as id, '
    . 'team.name as name, '
    . 'name_lc, '
    . 'name_html '
    . 'FROM scos_project, team '
    . 'WHERE active=1 '
    . 'AND scos_project.team = team.id');

while ($proj = mysql_fetch_object($result)) {
    $friendly_name = $proj->name_html;
    if (!$friendly_name) {
	$friendly_name = $proj->name;
    }
    if (!$friendly_name) {
	$friendly_name = "?";
    }
    if ($xml) {
        echo "  <scos_project>\n";
        echo "    <id>$proj->id</id>\n";
        echo "    <name>$proj->name_lc</name>\n";
    } else {
        echo "
            <tr>
              <td><a name='$proj->name_lc'>$friendly_name</a></td>
        ";

	$r2 = mysql_query("SELECT count(*) as num "
		. "FROM scos_result, scos_tool "
		. "WHERE scos_result.tool = scos_tool.id "
		. "AND project=$proj->id");
	if ($av = mysql_fetch_object($r2)) {
	   echo "
		  <td>
                    <a href='results.php?projid=$proj->id'>
                      $av->num
                    </a>
                  </td>
	   ";
	} else {
	   echo "
		  <td>".tr(SCOSP_NO_RESULTS)."</td>
	   ";
	}

	$r3 = mysql_query("SELECT date "
		. "FROM scos_result, scos_tool "
		. "WHERE scos_result.tool = scos_tool.id "
		. "AND project=$proj->id "
		. "ORDER BY date DESC "
		. "LIMIT 1");
	if ($av = mysql_fetch_object($r3)) {
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
