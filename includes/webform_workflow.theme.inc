<?php
/**
 * @file
 * Theme functions for the Webform Workflow module.
 */

/**
 * Theme function for the state table.
 */
function theme_webform_workflow_admin_form($variables) {
  $form = $variables['element'];

  $rows = array();
  foreach (element_children($form['items']) as $key) {
    $rows[] = array(
      'data' => array(
        drupal_render($form['items'][$key]['label']),
        drupal_render($form['items'][$key]['view_permissions']),
        drupal_render($form['items'][$key]['edit_permissions']),
        drupal_render($form['items'][$key]['from_permissions']),
        drupal_render($form['items'][$key]['to_permissions']),
        drupal_render($form['items'][$key]['notify_users']),
        drupal_render($form['items'][$key]['weight']),
      ),
      'class' => array('draggable'),
    );
  }

  $table_id = 'webform-workflow-states';

  $header = array(
    t('State'),
    t('View Permissions'),
    t('Edit Permissions'),
    t('Transition From'),
    t('Transition To'),
    t('Notify users/emails'),
    t('Weight'),
  );

  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
  ));
  $output .= drupal_render_children($form);

  drupal_add_tabledrag($table_id, 'order', 'sibling', 'state-item-weight');
  return $output;
}
