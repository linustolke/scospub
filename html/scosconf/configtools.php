<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/translation.inc");

init_session();
db_init();

$projid=$_REQUEST["project"];

$commit = 0;

$result = mysql_query("SELECT name FROM scos_tool "
	. "WHERE project=1 AND active=1 GROUP BY name");
while ($tool = mysql_fetch_object($result)) {
    $value = $_REQUEST[$tool->name];
    if ($value) {
        mysql_query("DELETE FROM scos_tool "
	    . "WHERE name='$tool->name' AND project=$projid");
	if ($value != "off") {
	    mysql_query("INSERT INTO scos_tool "
		. "SET project='$projid', name='$tool->name', config='$value', active=1");
	}

	$commit++;
    }
}

if ($commit > 0) {
    header("Location: projects.php#proj$projid");
    exit;
}



page_head(tr(SCOSC_TITLE));

$result = mysql_query("SELECT * FROM scos_project WHERE id=$projid");
$proj = mysql_fetch_object($result);
mysql_free_result($result);

echo tr(SCOSC_CONFIGURE_TOOLS_DESCRIPTION)."<p>
$proj->name ($proj->user_friendly_name):";

// Lets put the current configuration in an array.
$result = mysql_query("SELECT * FROM scos_tool WHERE project=$projid");
$enabled_tools = array();
$enabled_tools_config = array();
while ($tool = mysql_fetch_object($result)) {
    $enabled_tools[$tool->name] = 1;

    // 2 if active, 1 if not active
    $enabled_tools_config[$tool->name.", ".$tool->config] = 1 + $tool->active;
}
mysql_free_result($result);


echo "<FORM METHOD='get' ACTION=''>
";
echo "<INPUT type='hidden' name='project' value='$projid'>
";
echo "<UL>
";

// Now go through the template and generate a page.
$result = mysql_query("SELECT name FROM scos_tool WHERE project=1 AND active=1 GROUP BY name");
while ($tool = mysql_fetch_object($result)) {
    echo "<LI>$tool->name:
        <UL>
            <LI><INPUT type='radio' name='$tool->name' value='off' ";
    if ($enabled_tools[$tool->name] == 0) {
        echo 'checked="true"';
    }
    echo ">".tr(SCOSC_TOOL_NOT_ENABLED)."</LI>
    ";
    $configresult = mysql_query("SELECT * FROM scos_tool WHERE project=1 AND active=1 AND name='$tool->name'");
    while ($config = mysql_fetch_object($configresult)) {
	echo "<LI><INPUT type='radio' name='$tool->name' value='$config->config' ";
        if ($enabled_tools_config[$config->name.", ".$config->config] != 0) {
            echo 'checked="1"';
        }
        echo ">$config->config</LI>
        ";
    }
    mysql_free_result($configresult);

    $configresult = mysql_query("SELECT * FROM scos_tool "
	. "WHERE project=$projid "
	. "AND name='$tool->name' "
	. "AND config NOT IN (SELECT config FROM scos_tool WHERE name='$tool->name' AND project=1)");
    if (mysql_numrows($configresult) > 0) {
        echo tr(SCOSC_TOOLS_CONFIG_NOT_AVAILABLE).":<UL>
        ";
        while ($config->mysql_fetch_object($configresult)) {
            echo "<LI>$config->name $config->config</LI>
            ";
        }
        echo "</UL>
        ";
    }
    mysql_free_result($configresult);

    echo "</UL>
    </LI>
    ";
}
mysql_free_result($result);

echo "</UL>
";
echo "<INPUT type='submit' value='Submit'/>
";
echo "</FORM>
";


$result = mysql_query("SELECT * FROM scos_tool WHERE project=$projid AND name NOT IN (SELECT name FROM scos_tool WHERE project=1)");
if (mysql_numrows($result) > 0) {
    echo tr(SCOSC_TOOLS_NOT_AVAILABLE).":<UL>
    ";
    while ($tool->mysql_fetch_object($result)) {
        echo "<LI>$tool->name $tool->config</LI>
	";
    }
    echo "</UL>
    ";
}
mysql_free_result($result);

page_tail();
?>
