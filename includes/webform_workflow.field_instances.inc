<?php
/**
 * @file
 * Default field instances for the Webform Workflow module.
 */

/**
 * Get default field instances.
 *
 * @return array
 *   An array of field instance definitions.
 */
function _webform_workflow_field_instances() {
  $field_instances = array();

  // Exported field_instance: 'node-webform-webform_workflow_states'
  $field_instances['node-webform-webform_workflow_states'] = array(
    'bundle' => 'webform',
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'webform_workflow_states',
    'label' => 'States',
    'required' => 0,
    'settings' => array(
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'inline_entity_form',
      'settings' => array(
        'fields' => array(),
        'type_settings' => array(
          'allow_existing' => 0,
          'delete_references' => 1,
          'match_operator' => 'CONTAINS',
        ),
      ),
      'type' => 'inline_entity_form',
      'weight' => -4,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('States');

  return $field_instances;
}
