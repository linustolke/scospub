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
    $result = mysql_query("select * from scos_project where id=$projid");

    while ($proj = mysql_fetch_object($result)) {
	if ($xml) {
	    echo "  <id>$proj->id</id>\n";
	    echo "  <name>$proj->name</name>\n";
	    echo "  <results>\n";
	} else {
	    echo "  <b>$proj->name:</b>\n";
	    start_table();
	    echo "
		<tr>
		  <th>".tr(SCOSP_TOOL)."</th>
		  <th>".tr(SCOSP_VERSION)."</th>
		  <th>".tr(SCOSP_RESULT)."</th>
		</tr>
	    ";
        }

        $r2 = mysql_query('SELECT '
		.'scos_tools.name as name, '
		.'scos_tools.id as toolid, '
		.'scos_results.version as version, '
		.'scos_results.result as result '
		.'FROM scos_tools, scos_results '
		.'WHERE scos_tools.id = scos_result.tool '
		."AND projectid = $proj->id "
		.'ORDER BY version, scos_tools.name ');
	while ($res = mysql_fetch_object($r2)) {
	    if ($xml) {
                echo "  <result>\n";
                echo "    <tool>$res->toolid</tool>\n";
		echo "    <version>$res->version</version>\n";
		echo "    <status>$res->result</status>\n";
                echo "  </result>\n";
            } else {
		echo "
		     <tr>
		       <td>
			 $res->name
		       </td>
		       <td>
			 $res->version
		       </td>
		       <td>
			 $res->result
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
