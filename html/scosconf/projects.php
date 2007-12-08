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
} else {
    page_head(tr(SCOSC_TITLE));
    echo tr(SCOSC_DESCRIPTION)."<p>
    ";
}
$result = mysql_query("SELECT * FROM scos_project WHERE id > 1");

if (mysql_numrows($result) > 0) {
    if ($xml) {
        echo "<scos_projects>\n";
    } else {
	start_table();
	echo "
	    <tr>
	      <th>".tr(SCOSC_PROJECTNAME)."</th>
	      <th>".tr(SCOSP_THEPROJECT)."</th>
	      <th>".tr(SCOSC_SOURCES)."</th>
	      <th>".tr(SCOSC_TOOLS)."</th>
	    </tr>
	";
    }
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
		  <td><a name='proj$proj->id'>$proj->name</a>
	    ";
	    if ($proj->active) {
		echo tr(SCOSC_ACTIVE);
	    } else {
		echo tr(SCOSC_NOT_ACTIVE);
	    }
	    echo "
		  </td>
		  <td>$friendly_name</td>
	    ";

	    echo "
		  <td>
	    ";
	    $r2 = mysql_query('SELECT * '
		    . 'FROM scos_source '
		    . "WHERE project = $proj->id");

	    if (mysql_numrows($r2) > 0) {
		echo "
		    <ul>
		";
		while ($av = mysql_fetch_object($r2)) {
		    echo "<li>";
		    if ($av->type == 1) {
			echo "<a href='subversionsrc.php?source=$av->id'>";
			echo tr(SCOSC_SVN_SOURCE_URL);
			echo " $av->url</a>";
		    } else {
			echo "".tr(SCOSC_UNKNOWN_TYPE)."";
		    }
		    echo "</li>";
		}
		echo "
		    </ul>
		";
	    } else {
		echo tr(SCOSC_NO_SOURCE);
	    }
	    echo "
		  </td>
	    ";

	    echo "
		  <td>
	    ";
	    $r3 = mysql_query('SELECT * '
		    . 'FROM scos_tool '
		    . "WHERE project = $proj->id "
		    . 'AND project > 1');

	    if (mysql_numrows($r3) > 0) {
		echo "
		    <ul>
		";
		while ($av = mysql_fetch_object($r3)) {
		    echo "<li>";
		    echo "<a href='configtools.php?project=$proj->id'>
			".tr(SCOSC_CONFIG_TOOLS)."</a>
		    ";
		    echo " $av->name";
		    echo " $av->config ";
		    if ($av->active) {
			echo tr(SCOSC_ACTIVE);
		    } else {
			echo tr(SCOSC_NOT_ACTIVE);
		    }
		    echo "</li>";
		}
		echo "
		    </ul>
		";
	    } else {
		echo "<a href='configtools.php?project=$proj->id'>"
		    .tr(SCOSC_NO_TOOL)."</a>
		";
	    }
	    echo "
		  </td>
	    ";

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
    }
} else {
    echo tr(SCOSC_NO_PROJECTS)."<p>
    ";
}

if ($xml) {
} else {
    echo tr(SCOSC_PROJECT_HELP)."<p>
    ";
    page_tail();
}
?>
