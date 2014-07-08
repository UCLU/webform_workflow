<?php
/**
 * @file
 * webform_workflow_state_field.features.field_base.inc
 */

/**
 * Implements hook_field_default_field_bases().
 */
function webform_workflow_state_field_field_default_field_bases() {
  $field_bases = array();

  // Exported field_base: 'field_ww_states'
  $field_bases['field_ww_states'] = array(
    'active' => 1,
    'cardinality' => -1,
    'deleted' => 0,
    'entity_types' => array(),
    'field_name' => 'field_ww_states',
    'foreign keys' => array(
      'webform_workflow_states' => array(
        'columns' => array(
          'target_id' => 'wsid',
        ),
        'table' => 'webform_workflow_states',
      ),
    ),
    'indexes' => array(
      'target_id' => array(
        0 => 'target_id',
      ),
    ),
    'locked' => 0,
    'module' => 'entityreference',
    'settings' => array(
      'handler' => 'base',
      'handler_settings' => array(
        'behaviors' => array(
          'views-select-list' => array(
            'status' => 0,
          ),
        ),
        'sort' => array(
          'type' => 'none',
        ),
        'target_bundles' => array(),
      ),
      'target_type' => 'webform_workflow_state',
    ),
    'translatable' => 0,
    'type' => 'entityreference',
  );

  return $field_bases;
}
