<?php

/**
 * @file
 * User page callbacks for the token module.
 */

/**
 * Theme a link to a token tree either as a regular link or a dialog.
 */
function theme_token_tree_link($variables) {
  if (empty($variables['text'])) {
    $variables['text'] = t('Browse available tokens.');
  }

  if (!empty($variables['dialog'])) {
    backdrop_add_library('system', 'backdrop.ajax');
    $variables['options']['attributes']['class'][] = 'use-ajax';
  }

  $info = token_theme();
  $tree_variables = array_intersect_key($variables, $info['token_tree']['variables']);
  $tree_variables = backdrop_array_diff_assoc_recursive($tree_variables, $info['token_tree']['variables']);
  if (!isset($variables['options']['query']['options'])) {
    $variables['options']['query']['options'] = array();
  }
  $variables['options']['query']['options'] += $tree_variables;

  // We should never pass the dialog option to theme_token_tree(). It is only
  // used for this function.
  unset($variables['options']['query']['options']['dialog']);

  // Add a security token so that the tree page should only work when used
  // when the dialog link is output with theme('token_tree_link').
  $variables['options']['query']['token'] = backdrop_get_token('token-tree:' . serialize($variables['options']['query']['options']));

  // Because PHP converts query strings with arrays into a different syntax on
  // the next request, the options have to be encoded with JSON in the query
  // string so that we can reliably decode it for token comparison.
  $variables['options']['query']['options'] = backdrop_json_encode($variables['options']['query']['options']);

  // Set the token tree to open in a separate window.
  $variables['options']['attributes'] + array('target' => '_blank');

  return l($variables['text'], 'token/tree', $variables['options']);
}

/**
 * Page callback to output a token tree as an empty page.
 */
function token_page_output_tree() {
  $options = isset($_GET['options']) ? backdrop_json_decode($_GET['options']) : array();

  // Check the token against the serialized options to prevent random access to
  // the token browser page.
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'token-tree:' . serialize($options))) {
    return MENU_ACCESS_DENIED;
  }

  // Return as a renderable so the Backdrop dialog system can use the title.
  $tree = '<div>' . theme('token_tree', $options) . '</div>';
  if (backdrop_is_ajax()) {
    $dialog_options = array(
      'dialogClass' => 'token-dialog',
      'modal' => FALSE,
      'draggable' => TRUE,
      'resizeable' => TRUE,
      'autoResize' => FALSE,
      'width' => '50%',
      'height' => '600',
    );
    $commands = array();
    $commands[] = ajax_command_open_dialog('#token-dialog', t('Available tokens'), $tree, $dialog_options);
    $return = array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
    return $return;
  }
  else {
    return $tree;
  }
}

/**
 * Theme a tree table.
 *
 * @ingroup themeable
 */
function theme_tree_table($variables) {
  foreach ($variables['rows'] as &$row) {
    $row += array('class' => array());
    if (!empty($row['id'])) {
      $row['data-tt-id'] = $row['id'];
      $row['no_striping'] = TRUE;
      unset($row['id']);
    }
    if (!empty($row['parent'])) {
      $row['data-tt-parent-id'] = $row['parent'];
      unset($row['parent']);
    }
  }

  if (!empty($variables['rows'])) {
    backdrop_add_library('token', 'jquery.treetable');
  }

  return theme('table', $variables);
}

/**
 * Provide a 'tree' display of nested tokens.
 *
 * @ingroup themeable
 */
function theme_token_tree($variables) {
  $token_types = $variables['token_types'];
  $info = token_get_info();

  if ($token_types == 'all') {
    $token_types = array_keys($info['types']);
  }
  elseif ($variables['global_types']) {
    $token_types = array_merge($token_types, token_get_global_token_types());
  }

  $options = array(
    'flat' => TRUE,
    'restricted' => $variables['show_restricted'],
    'depth' => $variables['recursion_limit'],
  );
  $multiple_token_types = (count($token_types) > 1);
  $rows = array();

  foreach ($info['types'] as $type => $type_info) {
    if (!in_array($type, $token_types)) {
      continue;
    }

    if ($multiple_token_types) {
      $row = _token_token_tree_format_row($type, $type_info, TRUE);
      unset($row['data']['value']);
      $rows[] = $row;
    }

    $tree = token_build_tree($type, $options);
    foreach ($tree as $token => $token_info) {
      if (!empty($token_info['restricted']) && empty($variables['show_restricted'])) {
        continue;
      }
      if ($multiple_token_types && !isset($token_info['parent'])) {
        $token_info['parent'] = $type;
      }
      $row = _token_token_tree_format_row($token, $token_info);
      unset($row['data']['value']);
      $rows[] = $row;
    }
  }

  $table_variables = array(
    'header' => array(
      t('Name'),
      t('Token'),
    ),
    'rows' => $rows,
    'attributes' => array('class' => array('token-tree')),
    'empty' => t('No tokens available'),
  );

  if ($variables['click_insert']) {
    $table_variables['caption'] = t('Select a token to insert it into the current field.');
    $table_variables['attributes']['class'][] = 'token-click-insert';
  }

  backdrop_add_library('token', 'token');
  $output = theme('tree_table', $table_variables);
  return $output;
}

/**
 * Build a row in the token tree.
 */
function _token_token_tree_format_row($token, array $token_info, $is_group = FALSE) {
  // Build a statically cached array of default values. This is around four
  // times as efficient as building the base array from scratch each time this
  // function is called.
  static $defaults = array(
    'id' => '',
    'class' => array(),
    'data' => array(
      'name' => '',
      'token' => '',
      'value' => '',
    ),
  );

  $row = $defaults;
  $row['id'] = backdrop_clean_css_identifier($token);
  $row['data']['name']['data'] = $token_info['name'];
  $row['data']['name']['class'][] = 'token-name';

  if ($is_group) {
    // This is a token type/group.
    $row['class'][] = 'token-group';
    $row['data']['token']['data'] = '<span class="token-group-description">' . $token_info['description'] . '</span>';
  }
  else {
    // This is a token.
    $row['data']['token']['class'][] = 'token-token';
    $row['data']['token']['data'] = '<span class="token-key">' . $token . '</span>';
    if (isset($token_info['value'])) {
      $row['data']['value'] = $token_info['value'];
    }
    if (!empty($token_info['parent'])) {
      $row['parent'] = backdrop_clean_css_identifier($token_info['parent']);
    }
    $row['data']['token']['data'] .= '<span class="token-description">' . $token_info['description'] . '</span>';
  }

  return $row;
}

/**
 * Menu callback; prints the available tokens and values for an object.
 */
function token_devel_token_object($entity_type, $entity, $token_type = NULL) {
  $header = array(
    t('Token'),
    t('Value'),
  );
  $rows = array();

  $options = array(
    'flat' => TRUE,
    'values' => TRUE,
    'data' => array($entity_type => $entity),
  );
  if (!isset($token_type)) {
    $token_type = $entity_type;
  }
  $tree = token_build_tree($token_type, $options);
  foreach ($tree as $token => $token_info) {
    if (!empty($token_info['restricted'])) {
      continue;
    }
    if (!isset($token_info['value']) && !empty($token_info['parent']) && !isset($tree[$token_info['parent']]['value'])) {
      continue;
    }
    $row = _token_token_tree_format_row($token, $token_info);
    unset($row['data']['name']);
    $rows[] = $row;
  }

  $build['tokens'] = array(
    '#theme' => 'tree_table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('class' => array('token-tree')),
    '#empty' => t('No tokens available.'),
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'token') . '/token.js'),
      'css' => array(backdrop_get_path('module', 'token') . '/token.css'),
    ),
  );

  return $build;
}

/**
 * Page callback to clear the token registry caches.
 */
function token_flush_cache_callback() {
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], current_path())) {
    return MENU_NOT_FOUND;
  }

  token_clear_cache();
  backdrop_set_message(t('Token registry caches cleared.'));
  backdrop_goto();
}

function token_autocomplete() {
  $args = func_get_args();
  $string = implode('/', $args);

  $token_info = token_info();

  preg_match_all('/\[([^\s\]:]*):?([^\s\]]*)?\]?/', $string, $matches);
  $types = $matches[1];
  $tokens = $matches[2];

  foreach ($types as $index => $type) {
    if (!empty($tokens[$index]) || isset($token_info['types'][$type])) {
      token_autocomplete_token($type, $tokens[$index]);
    }
    else {
      token_autocomplete_type($type);
    }
  }

}

function token_autocomplete_type($string = '') {
  $token_info = token_info();
  $types = $token_info['types'];
  $matches = array();

  foreach ($types as $type => $info) {
    if (!$string || strpos($type, $string) === 0) {
      $type_key = "[{$type}:";
      $matches[$type_key] = levenshtein($type, $string);
    }
  }

  if ($string) {
    asort($matches);
  }
  else {
    ksort($matches);
  }

  $matches = backdrop_map_assoc(array_keys($matches));
  backdrop_json_output($matches);
}

function token_autocomplete_token($token_type) {
  $args = func_get_args();
  array_shift($args);
  $string = trim(implode('/', $args));
  $string = substr($string, strrpos($string, '['));

  $token_type = $token_type['type'];
  $matches = array();

  if (!backdrop_strlen($string)) {
    $matches["[{$token_type}:"] = 0;
  }
  else {
    $depth = max(1, substr_count($string, ':'));
    $tree = token_build_tree($token_type, array('flat' => TRUE, 'depth' => $depth));
    foreach (array_keys($tree) as $token) {
      if (strpos($token, $string) === 0) {
        $matches[$token] = levenshtein($token, $string);
        if (isset($tree[$token]['children'])) {
          $token = rtrim($token, ':]') . ':';
          $matches[$token] = levenshtein($token, $string);
        }
      }
    }
  }

  asort($matches);
  $matches = backdrop_map_assoc(array_keys($matches));
  backdrop_json_output($matches);
}
