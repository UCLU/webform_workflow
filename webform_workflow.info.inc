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
    'weight' => array(
      'label' => t('Weight'),
      'description' => t('The weight of the state'),
      'type' => 'integer',
      'schema field' => 'weight',
      'required' => TRUE,
    ),
  );
  return $info;
}
