<?php
/**
 * glFusion API functions for the Searcher plugin.
 *
 * @author      Lee Garner <lee@leegarner.com>
 * @copyright   Copyright (c) 2017-2020 Lee Garner <lee@leegarner.com>
 * @package     searcher
 * @version     v1.1.0
 * @license     http://opensource.org/licenses/gpl-2.0.php
 *              GNU Public License v2 or later
 * @filesource
 */

if (!defined('GVERSION')) {
    die('This file can not be used on its own.');
}

/** Import configuration-handling class */
$searcher_config = \config::get_instance();
$_SRCH_CONF = $searcher_config->get_config('searcher');

/** Additional config items and table definitions are added here */
require_once __DIR__ . '/searcher.php';
define('SRCH_PI_PATH', __DIR__);
define('SRCH_URL', $_CONF['site_url'] . '/' . $_SRCH_CONF['pi_name']);
define('SRCH_ADMIN_URL', $_CONF['site_admin_url'] . '/plugins/' . $_SRCH_CONF['pi_name']);

$_SRCH_CONF['_is_uikit'] = $_SYSTEM['framework'] == 'uikit' ? true : false;

/**
 * Include the plugin-specific language file, and the corresponding
 * admin-created custom language file, if any.  Looks for the site
 * language first, then defaults to english_utf-8.php if nothing else found.
 */
$langpath = SRCH_PI_PATH . '/language';
if (is_file("$langpath/{$_CONF['language']}.php")) {
    $langfile = "{$_CONF['language']}.php";
} else {
    $langfile = 'english_utf-8.php';
}
require_once "$langpath/$langfile";

if (is_file("$langpath/custom/$langfile")) {
    /** Include any custom language file, if found */
    include_once "$langpath/custom/$langfile";
}

/** Include the user-supplied configuration file, if any */
if (is_file(SRCH_PI_PATH . '/config.php')) {
    include_once SRCH_PI_PATH . '/config.php';
}

// Replace the stock glFusion search, if so configured
if ( isset($_SRCH_CONF['replace_stock_search']) && $_SRCH_CONF['replace_stock_search'] == true ) {
    PLG_registerService('search','\\Searcher\\Searcher');
}

/* Auto loader */
spl_autoload_register(function ($class)
{
    // project-specific namespace prefix
    $prefix = 'Searcher\\';

    // does the class use the namespace prefix?
    $len = strlen($prefix);
    if (strncmp($prefix, $class, $len) !== 0) {
        // no, move to the next registered autoloader
        return;
    }

    // base directory for the namespace prefix
    $base_dir = __DIR__ . '/classes/';

    // get the relative class name
    $relative_class = substr($class, $len);

    // replace the namespace prefix with the base directory, replace namespace
    // separators with directory separators in the relative class name, append
    // with .php
    $file = $base_dir . str_replace('\\', '/', $relative_class) . '.class.php';

    // if the file exists, require it
    if (file_exists($file)) {
        require $file;
    }
});


/**
 * Get the currently installed version.
 *
 * @return  string  version number
 */
function plugin_chkVersion_searcher()
{
    global $_SRCH_CONF;
    return $_SRCH_CONF['pi_version'];
}


/**
 * Puts an option for the plugin in the command and control block.
 * Not used in glFusion versions > 1.6.0 since the database backup function
 * has been updated in the main DB Backup option.
 *
 * @return  array  Array of (plugin name, admin url, url of plugin icon)
 */
function plugin_cclabel_searcher()
{
    global $LANG_SRCH, $_CONF;

    if (SEC_inGroup('Root')) {
        return array(
            $LANG_SRCH['menu_label'],
            SRCH_ADMIN_URL . '/index.php',
            plugin_geticon_searcher(),
        );
    } else {
        return '';
    }
}


/**
 * Adds the plugin to the Admin menu.
 *
 * @return  array   Array(pi_name, admin url, # of items) or '' for none.
 */
function plugin_getadminoption_searcher()
{
    global $_CONF, $LANG_SRCH;

    if (SEC_inGroup('Root')) {
        $retval = array(
            $LANG_SRCH['menu_label'],
            SRCH_ADMIN_URL . '/index.php',
            0,
        );
    } else {
        $retval = '';
    }
    return $retval;
}


/**
 * Returns the URL of the plugin's icon.
 *
 * @return  string      URL of the icon
 */
function plugin_geticon_searcher()
{
    return SRCH_URL . '/images/searcher.png';
}


/**
 * Get the link to a documentation file.
 * Returns the URL to the file, or to the directory if no file requested.
 *
 * @see     plugin_getconfigelementhelp_searcher()
 * @param   string  $file       HTML file
 * @param   string  $pi_name    Name of plugin, default "searcher"
 * @param   string  $doclang    Documentation language, default English
 * @return  string              URL to document directory or file
 */
function SRCH_getDocURL($file='', $pi_name='searcher', $doclang='english')
{
    global $_CONF, $_SRCH_CONF;

    $doc_path = "{$_CONF['path_html']}$pi_name/docs/$doclang/";
    $doc_url = '';

    if ($file != '') {
        // Split out any .html extension to insert ".legacy" if needed
        $fparts = pathinfo($file);
        $file = $fparts['filename'];
        if (version_compare(GVERSION, '1.6.0', '<')) {
            $file .= '.legacy';
        }
        $file .= '.html';

        if (!is_file($doc_path . $file)) {
            $doclang = 'english';
        }
        $doc_url = "{$_CONF['site_url']}/$pi_name/docs/$doclang/$file";
    } else {
        if (!is_dir($doc_path)) {
            $doclang = 'english';
        }
        $doc_url = "{$_CONF['site_url']}/$pi_name/docs/$doclang/";
    }
    return $doc_url;
}


/**
 * Get the URL to the configuration help file.
 *
 * @param   string  $option     Config option selected
 * @param   string  $doclang    Document language
 * @return  string              URL to document
 */
function plugin_getconfigelementhelp_searcher($option, $doclang='english')
{
    global $_SRCH_CONF;

    $file = 'config.html';

    $doc_url = SRCH_getDocURL($file, $_SRCH_CONF['pi_name'], $doclang);

    if (empty($doc_url))
        return false;

    if (!empty($option)) {
        $doc_url .= '#desc_' . $option;
    }

    return array($doc_url, 0);
}


/**
 * Upgrade the plugin to the currently-installed version.
 *
 * @return  boolean     True if successful, False otherwise
 */
function plugin_upgrade_searcher()
{
    require_once __DIR__ . '/upgrade.inc.php';
    return SRCH_do_upgrade();
}


/**
 * Get additional CSS into the site header.
 * Allows for custom CSS to be placed by the site admin.
 *
 * @return  array   List of paths to CSS files
 */
function plugin_getheadercss_searcher()
{
    global $_CONF, $_SRCH_CONF;

    $styles = array();
    $files = array(
        'style.css',
    );

    // When looking for CSS files, we'll check in this order:
    //  1. layout/plugin/custom
    //  2. layout/plugin
    //  3. plugin/custom
    //  4. plugin (default distribution version)
    $pi_css = SRCH_PI_PATH . '/css/';
    $layout_css = "{$_CONF['path_layout']}plugins/{$_SRCH_CONF['pi_name']}/";
    foreach ($files as $file) {
        if (@file_exists($layout_css . 'custom/' . $file)) {
            $styles[] = $layout_css . 'custom/' . $file;
        } elseif (@file_exists($layout_css . $file)) {
            $styles[] = $layout_css . $file;
        } elseif (@file_exists($pi_css . 'custom/' . $file)) {
            $styles[] = $pi_css . 'custom/' . $file;
        } elseif (@file_exists($pi_css . $file)) {
            $styles[] = $pi_css . $file;
        }
    }
    return $styles;
}


/**
 * Index an item when it is saved.
 * First deletes any existing index records, then creates newones.
 *
 * @param   string  $id     Item ID
 * @param   string  $type   Item Type
 * @param   string  $old_id Original ID, in case ID has changed
 */
function plugin_itemsaved_searcher($id, $type, $old_id = '')
{
    global $_TABLES;

    $contentInfo = PLG_getItemInfo(
        $type, $id,
        'id,date,parent_type,parent_id,title,searchidx,author,author_name,hits,perms,search_index,status',
        2
    );

    // Document is always removed before indexing anyway,
    // just remove it here in case contentInfo is invalid.
    if ( $old_id != '' && $id != $old_id ) {
        \Searcher\Indexer::RemoveDoc($type, $old_id);
    }
    \Searcher\Indexer::RemoveDoc($type, $id);

    if (
        is_array($contentInfo) &&
        count($contentInfo) > 0 &&
        isset($contentInfo['searchidx']) &&
        !empty($contentInfo['searchidx'])
    ) {
        if ($type == 'comment') {
            // For comments, get the parent item's permissions as "root".
            $parent = PLG_getItemInfo(
                $contentInfo['parent_type'],
                $contentInfo['parent_id'],
                'id,perms',
                2
            );
            if (is_array($parent) && isset($parent['perms']) && is_array($parent['perms'])) {
                $contentInfo['perms'] = $parent['perms'];
            }
        }

        // If no permissions returned, use defaults
        if (!isset($contentInfo['perms']) || empty($contentInfo['perms'])) {
            $contentInfo['perms'] = array(
                'owner_id' => 2,
                'group_id' => 1,
                'perm_owner' => 3,
                'perm_group' => 2,
                'perm_members' => 2,
                'perm_anon' => 2,
            );
        }
        // If an "enabled" status isn't returned by the plugin, assume enabled
        if (!isset($contentInfo['status']) || is_null($contentInfo['status'])) {
            $contentInfo['status'] = 1;
        }

        $props = array(
            'item_id' => $contentInfo['id'],
            'type'  => $type,
            'author' => $contentInfo['author'],
            'author_name' => $contentInfo['author_name'],
            // Hack to avoid indexing comment titles which don't show anyway
            'title' => $type == 'comment' ? NULL : $contentInfo['title'],
            'content' => $contentInfo['searchidx'],
            'date' => $contentInfo['date'],
            'perms' => $contentInfo['perms'],
            'parent_id' => $contentInfo['parent_id'],
            'parent_type' => $contentInfo['parent_type'],
        );
        if ($contentInfo['status']) {
            // Index only if status is nonzero (i.e. not draft or disabled)
            \Searcher\Indexer::IndexDoc($props);
        }
    }
}


/**
 * Delete index records for a deleted item.
 *
 * @param   string  $id     Item ID
 * @param   string  $type   Item Type, e.g. plugin name
 * @param   string  $children   Optional comma-separated values to delete
 */
function plugin_itemdeleted_searcher($id, $type, $children='')
{
    if (!empty($children)) {
        $id = explode(',', $children);
    }
    \Searcher\Indexer::RemoveDoc($type, $id);
}


/**
 * PLG function to index a single document.
 *
 * @param   array   $args       Args, including type, item_id, title, etc.
 * @return  boolean     True on success, False on failure
 */
function plugin_indexDoc_searcher($args)
{
    // Check that the minimum required fields are set
    if (!isset($args['item_id']) || !isset($args['type']) ||
        (!isset($args['content']) && !isset($args['title']) && !isset($args['author']))
    ) {
        return false;
    }
    \Searcher\Indexer::RemoveDoc($args['type'], $args['item_id']);
    return \Searcher\Indexer::IndexDoc($args);
}


/**
 * PLG function to remove an item from the index.
 * Makes sure that a valid type and item_id are set to remove one item.
 *
 * @param   string  $type       Type of document
 * @param   string  $item_id    ID of doucment
 * @return  boolean     True on success, False on error
 */
function plugin_RemoveDoc_searcher($type, $item_id)
{
    if (\Searcher\Indexer::RemoveDoc($type, $item_id)) {
        \Searcher\Indexer::RemoveComments($type, $item_id);
         return true;
    } else {
        return false;
    }
}


/**
 * PLG function to remove all items for a plugin from the index.
 * May be called during plugin removal.
 * Makes sure that a valid type is set to remove all items for a single type.
 *
 * @param   string  $type       Type of item to remove
 * @return  integer     PLG_RET_OK on success, PLG_RET_ERROR on error
 */
function plugin_removeAll_searcher($type)
{
    // Check that the minimum required fields are set. Don't allwow plugins
    // to accidentally delete all.
    if (empty($type) || $type == 'all') {
        return false;
    }
    if (\Searcher\Indexer::RemoveAll($type)) {
        \Searcher\Indexer::RemoveComments($type);
        return true;
    }
    return false;
}


/**
 * Selection dropdown to pick the stemmer in the configuration manager.
 *
 * @return  Array   Associative array of section_name=>section_id
 */
function plugin_configmanager_select_stemmer_searcher()
{
    global $LANG_SRCH;

    $A = array($LANG_SRCH['none'] => '');
    // Collect the available stemmers
    $results = @glob(__DIR__ . '/classes/Stemmers/*.class.php');
    $installable = '';
    if (is_array($results)) {
        foreach ($results as $fullpath) {
            $parts = explode('/', $fullpath);
            list($class,$x1,$x2) = explode('.', $parts[count($parts)-1]);
            $A[$class] = $class;
        }
    }
    return $A;
}


/**
 * Reindex all comments for the given type.
 *
 * @param   string  $type   Content type, e.g. "article", "staticpages"
 * @param   mixed   $pid    Parent content item ID
 * @param   mixed   $perms  Permission array from content item, or NULL
 * @return  integer     Count of articles indexed
 */
function plugin_IndexAll_comments($type, $pid, $perms=NULL)
{
    global $_TABLES;

    if ( ! \Searcher\Indexer::CommentsEnabled() ) {
        return 0;
    }

    $type = DB_escapeString($type);
    $pid = DB_escapeString($pid);
    $sql = "SELECT cid, sid, uid, title, comment, UNIX_TIMESTAMP(date) AS unixdate
        FROM {$_TABLES['comments']}
        WHERE type = '$type' AND sid = '$pid'";
    //echo $sql;die;
    $res = DB_query($sql);
    $count = 0;
    // Remove all existing index records for this content
    \Searcher\Indexer::RemoveComments($type, $pid);
    while ($A = DB_fetchArray($res, false)) {
        $count++;
        $props = array(
            'item_id' => $A['cid'],
            'parent_id' => $A['sid'],
            'parent_type' => $type,
            'type'  => 'comment',
            //'title' => $A['title'],
            'author' => $A['uid'],
            'content' => $A['comment'],
            'date' => $A['unixdate'],
        );
        if (is_array($perms)) {
            $props['perms'] = array(
                'owner_id' => $perms['owner_id'],
                'group_id' => $perms['group_id'],
                'perm_owner' => $perms['perm_owner'],
                'perm_group' => $perms['perm_group'],
                'perm_members' => $perms['perm_members'],
                'perm_anon' => $perms['perm_anon'],
            );
        }
        \Searcher\Indexer::IndexDoc($props);
    }
    return $count;
}

?>
