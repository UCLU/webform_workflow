<?php
/**
 * @file
 * Entity property information for Webform Workflow States.
 */

/**
 * Implements hook_entity_property_info().
 */
function webform_workflow_property_info() {
  $info = array();
  $properties = &$info['webform_workflow_state']['properties'];
  $properties = array(
    'wsid' => array(
      'label' => t('State ID'),
      'description' => t('Primary key'),
      'type' => 'integer',
      'schema field' => 'wsid',
      'required' => TRUE,
    ),
    'nid' => array(
      'label' => t('Webform'),
      'description' => t('The webform node associated with the state'),
      'type' => 'node',
      'schema field' => 'nid',
      'required' => TRUE,
    ),
    'label' => array(
      'description' => t('The name of the state'),
      'type' => 'text',
      'schema field' => 'label',
      'required' => TRUE,
    ),
    'color' => array(
      'description' => t('The state color'),
      'type' => 'text',
      'schema field' => 'color',
      'required' => FALSE,
      'options list' => 'webform_workflow_state_color_options_list',
    ),
  );
  return $info;
}
