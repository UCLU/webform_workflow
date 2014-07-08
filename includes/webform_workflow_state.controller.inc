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
      'data' => $this->getDataDefaults(),
      'weight' => 0,
    );
    // Overwrite default values with supplied values.
    $values = array_merge_recursive($default_values, $values);
    return parent::create($values);
  }

  /**
   * Get the defaults for the $state->data array.
   *
   * @return array
   */
  protected function getDataDefaults() {
    return array(
      'permissions' => array(
        'view' => array(),
        'edit' => array(),
        'from' => array(),
        'to' => array(),
      ),
      'notify_users' => array(),
    );
  }

  /**
   * @{inheritdoc}
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    $data_defaults = $this->getDataDefaults();
    foreach ($entities as $state) {
      if (is_string($state->data)) {
        $state->data = (array) unserialize($state->data);
      }
      $state->data = array_merge_recursive($data_defaults, $state->data);
    }
    return $entities;
  }

  /**
   * @{inheritdoc}
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    if (is_array($entity->data)) {
      $entity->data = serialize($entity->data);
    }
    return parent::save($entity, $transaction);
  }

}
