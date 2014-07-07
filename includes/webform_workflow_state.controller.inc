<?php
/**
 * @file
 * Entity controller for a webform workflow state.
 */

class WebformWorkflowStateController extends EntityAPIControllerExportable {

  /**
   * @{inheritdoc}
   */
  public function create(array $values = array()) {
    $default_values = array(
      'wsid' => NULL,
      'nid' => NULL,
      'label' => NULL,
      'data' => array(),
      'weight' => 0,
    );
    // Overwrite default values with supplied values.
    $values = array_merge($default_values, $values);
    return parent::create($values);
  }

  /**
   * @{inheritdoc}
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    foreach ($entities as $state) {
      if (!empty($state->data) && is_string($state->data)) {
        $state->data = (array) unserialize($state->data);
      }
    }
    return $entities;
  }

}
