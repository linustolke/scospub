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
	      <th>".tr(SCOSC_SOURCES)."<br>
	      ".tr(SCOSC_TOOLS)."</th>
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
		<TR>
		  <TD VALIGN='TOP' ROWSPAN='2'>
                    <A NAME='proj$proj->id' 
                       HREF='configproject.php?project=$proj->id'>
                      $proj->name
                    </A>
	    ";
	    if ($proj->active) {
		echo tr(SCOSC_ACTIVE);
	    } else {
		echo tr(SCOSC_NOT_ACTIVE);
	    }
	    echo "
		  </TD>
		  <TD VALIGN='TOP' ROWSPAN='2'>$friendly_name</TD>
	    ";

	    echo "
		  <TD VALIGN='TOP'>
	    ";
	    $r2 = mysql_query('SELECT * '
		    . 'FROM scos_source '
		    . "WHERE project = $proj->id");

	    if (mysql_numrows($r2) > 0) {
		echo "
		    <UL>
		";
		while ($av = mysql_fetch_object($r2)) {
		    // TODO: Change colors to notify invalid and unknown.
		    echo "<LI>";

		    if ($av->type == 1) {
			echo "<A HREF='configsource.php?project=$proj->id&id=$av->id'>";
			echo tr(SCOSC_SVN_SOURCE_URL);
			echo " $av->url</A> ($av->valid) ";
		    } else {
			echo "".tr(SCOSC_UNKNOWN_TYPE)." ";
		    }

		    if ($av->active) {
			echo tr(SCOSC_ACTIVE);
		    } else {
			echo tr(SCOSC_NOT_ACTIVE);
		    }

		    echo "</LI>";
		}
		echo "
		    </UL>
		";
	    } else {
		echo tr(SCOSC_NO_SOURCE);
	    }
            echo "<BR>
	          <A HREF='configsource.php?project=$proj->id'>"
		.tr(SCOSC_ADD_SOURCE)
		."</A>
	    ";
	    echo "
		  </TD>
                </TR>
	    ";

	    echo "
                <TR>
		  <TD VALIGN='TOP'>
	    ";
	    $r3 = mysql_query('SELECT * '
		    . 'FROM scos_tool '
		    . "WHERE project = $proj->id "
		    . 'AND project > 1');

	    if (mysql_numrows($r3) > 0) {
		echo "
		    <UL>
		";
		while ($av = mysql_fetch_object($r3)) {
		    echo "<LI>";
		    echo "<A HREF='configtools.php?project=$proj->id'>
			".tr(SCOSC_CONFIG_TOOLS)."</A>
		    ";
		    echo " $av->name";
		    echo " $av->config ";
		    if ($av->active) {
			echo tr(SCOSC_ACTIVE);
		    } else {
			echo tr(SCOSC_NOT_ACTIVE);
		    }
		    echo "</LI>";
		}
		echo "
		    </UL>
		";
	    } else {
		echo "<A HREF='configtools.php?project=$proj->id'>"
		    .tr(SCOSC_NO_TOOL)."</A>
		";
	    }
	    echo "
		  </TD>
	    ";

	    echo "
		</TR>
	    ";
	}

	if ($xml) {
	    echo "  </scos_project>\n";
	} else {
	    echo "
		</TR>
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
    echo tr(SCOSC_NO_PROJECTS)."<P>
    ";
}

if ($xml) {
} else {
    echo tr(SCOSC_PROJECT_HELP)."<p>
    ";

    echo "<A HREF='configproject.php'>".tr(SCOSC_NEW_PROJECT)."</A>";

    page_tail();
}
?>
