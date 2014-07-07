<?php
/**
 * @File: Rules integration
 */

/**
 * Implements hook_rules_event_info().
 *
 * Defines Rules events for Webform Workflow transitions.
 */
function webform_workflow_rules_event_info() {
  $items = array(
    'webform_workflow_transition' => array(
      'label' => t('After webform transitions to a new workflow state'),
      // @TODO: Complete help text.
      'help' => t('Help text for this rules event - TBC'),
      'group' => t('Webform'),
      'variables' => array(
        'webform' => array(
          'label' => t('Webform'),
          'type' => 'node',
          'bundle' => 'webform',
        ),
        'state_from' => array(
          'label' => t('Previous submission state'),
          'type' => 'webform_workflow_state',
        ),
        'state_to' => array(
          'label' => t('New submission state'),
          'type' => 'webform_workflow_state',
        ),
      ),
    ),
  );

  return $items;
}
