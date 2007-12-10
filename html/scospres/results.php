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
    echo "<scos_results>\n";
} else {
    page_head(tr(SCOSP_TITLE));
    echo tr(SCOSP_RESULT_DESCRIPTION)."<br><br>
    ";
}
$projid = $_GET['projid'];
if (!$projid) {
    echo tr(SCOSP_RESULT_NO_PROJ)."<br>
        <a href='projects.php'>".tr(SCOSP_ALL_PROJECTS)."</a><br>
    ";
} else {
    $result = mysql_query("SELECT * FROM scos_project WHERE id=$projid");

    while ($proj = mysql_fetch_object($result)) {
	if ($xml) {
	    echo "  <id>$proj->id</id>\n";
	    echo "  <name>$proj->name</name>\n";
	    echo "  <results>\n";
	} else {
	    echo "  <b><a href='projects.php#$proj->name'>$proj->name</a>:</b>\n";
	    start_table();
	    echo "
		<tr>
		  <th>".tr(SCOSP_TOOL)."</th>
		  <th>".tr(SCOSP_SOURCE)."</th>
		  <th>".tr(SCOSP_DATE)."</th>
		  <th>".tr(SCOSP_RESULT)."</th>
		</tr>
	    ";
        }

        $r2 = mysql_query('SELECT '
		. 'scos_tool.name AS name, '
		. 'scos_tool.id AS toolid, '
		. 'scos_result.id AS id, '
		. 'scos_result.date AS date, '
		. 'scos_result.result AS result, '
		. 'scos_result.file AS file '
		. 'FROM scos_tool, scos_result '
		. 'WHERE scos_tool.id = scos_result.tool '
		. "AND project = $proj->id "
		. 'ORDER BY scos_tool.name ');
	while ($res = mysql_fetch_object($r2)) {
	    if ($xml) {
                echo "  <result>\n";
                echo "    <tool>$res->toolid</tool>\n";
		echo "    <status>$res->result</status>\n";
                echo "  </result>\n";
            } else {
		echo "
		     <tr>
		       <td>
			 $res->name
		       </td>
		       <td>";

		// TODO: Hardcoded for SVN
		$r3 = mysql_query('SELECT '
			. 'rooturl, revision '
			. 'FROM scos_source, scos_result_source '
			. 'WHERE scos_source.id = scos_result_source.source '
			. "AND scos_result_source.result = $res->id "
			. "GROUP BY rooturl");
                while ($res3 = mysql_fetch_object($r3)) {
			echo "$res3->rooturl $res3->revision <br>";
		}
		echo "
	               </td>
		       <td>
			 $res->date
		       </td>
		       <td>
			 <a href='DATA/$res->file'>$res->result</a>
		       </td>
		     </tr>
		";
	    }
	}

	if ($xml) {
	    echo "  </results>\n";
	} else {
	    echo "
		</tr>
	    ";
	}
    }
    mysql_free_result($result);
}
if ($xml) {
    echo "</scos_projects>\n";
} else {
    end_table();
    page_tail();
}
?>