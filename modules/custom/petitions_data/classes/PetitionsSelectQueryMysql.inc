<?php

/**
 * @file
 * Contains PetitionsSelectQueryMysql.
 */

/**
 * MySQL petitions select query class.
 */
class PetitionsSelectQueryMysql extends PetitionsSelectQuery {

  /**
   * The query object for this query.
   *
   * @var EntityFieldQueryExtraFields
   */
  protected $query;

  /**
   * The unlimited (COUNT) query object for this query.
   *
   * @var EntityFieldQueryExtraFields
   */
  protected $countQuery;

  /**
   * The size of the unlimited resultset.
   *
   * @var int
   */
  protected $resultsetCount = 0;

  /**
   * OrderBy ASC/DESC mapping
   *
   * @var array
   */
  protected $orderByDirMap = array(
    self::SELECT_QUERY_ORDER_BY_ASC => 'ASC',
    self::SELECT_QUERY_ORDER_BY_DESC => 'DESC',
  );

  /**
   * OrderBy MySQL Field mapping
   *
   * Maps Order By constants to appropriate database column name.
   *
   * @var array
   *   An array containing 'field' and 'column' elements. For node columns, only
   *   'field' is required. For petition fields, 'field' and 'column' name
   *   are required.
   */
  protected $orderByFieldsMap = array(
    self::SELECT_QUERY_ORDER_BY_FIELD_ID => array('field' => 'nid'),
    self::SELECT_QUERY_ORDER_BY_FIELD_TITLE => array('field' => 'title'),
    self::SELECT_QUERY_ORDER_BY_FIELD_DATE_CREATED => array('field' => 'created'),
    self::SELECT_QUERY_ORDER_BY_FIELD_PUBLISHED => array('field' => 'field_timestamp_published', 'column' => 'value'),
    self::SELECT_QUERY_ORDER_BY_FIELD_SIGNATURE_COUNT => array('field' => 'field_petition_signature_count', 'column' => 'value'),
    self::SELECT_QUERY_ORDER_BY_FIELD_DATE_REACHED_PUBLIC => array('field' => 'field_timestamp_reached_public', 'column' => 'value'),
  );

  protected static $petitionMetricFieldsMap = array(
    PetitionItem::PETITION_SIGNATURE_COUNT_EMAIL_SIGNUPS => array(
      'table' => 'field_petition_sig_email_signups',
      'field' => 'field_petition_sig_email_signups_value',
      'title' => 'Email Signups',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_FREE_EMAILS => array(
      'table' => 'field_petition_sig_free_emails',
      'field' => 'field_petition_sig_free_emails_value',
      'title' => 'Free Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_FORWARDED_EMAILS => array(
      'table' => 'field_petition_sig_fwd_emails',
      'field' => 'field_petition_sig_fwd_emails_value',
      'title' => 'Forwarding Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_OPEN_EMAILS => array(
      'table' => 'field_petition_sig_open_emails',
      'field' => 'field_petition_sig_open_emails_value',
      'title' => 'Open Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_SHRED_EMAILS => array(
      'table' => 'field_petition_sig_shred_emails',
      'field' => 'field_petition_sig_shred_emails_value',
      'title' => 'Shred Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_SUBADDRESSED_EMAILS => array(
      'table' => 'field_petition_sig_subadr_emails',
      'field' => 'field_petition_sig_subadr_emails_value',
      'title' => 'Subaddressed Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_TIMEBOUND_EMAILS => array(
      'table' => 'field_petition_sig_tmebnd_emails',
      'field' => 'field_petition_sig_tmebnd_emails_value',
      'title' => 'Time Bound Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_TRASH_EMAILS => array(
      'table' => 'field_petition_sig_trash_emails',
      'field' => 'field_petition_sig_trash_emails_value',
      'title' => 'Trash Email Addresses',
    ),
    PetitionItem::PETITION_SIGNATURE_COUNT_UNIQUE_EMAIL_ACCOUNTS => array(
      'table' => 'field_petition_sig_unq_email_act',
      'field' => 'field_petition_sig_unq_email_act_value',
      'title' => 'Unique Email Accounts',
    ),
    PetitionItem::PETITION_VALIDATION_COUNT_UNIQUE_IP_EMAILS => array(
      'table' => 'field_petition_val_unq_ip_emails',
      'field' => 'field_petition_val_unq_ip_emails_value',
      'title' => 'Unique Email IP Addresses',
    ),
  );

  /**
   * Constructor.
   */
  public function __construct() {
    parent::__construct();
    $this->setOrderBy(self::SELECT_QUERY_ORDER_BY_FIELD_DATE_CREATED, self::SELECT_QUERY_ORDER_BY_DESC);
  }

  /**
   * {@inheritdoc}
   */
  public function execute() {
    try {
      $this->buildQuery();
      $nodes = $this->executeQuery();
      $this->buildResults($nodes);
    }
    catch (PetitionNotFoundException $e) {
      $this->setResult(array());
    }
    catch (Exception $e) {
      watchdog('petitions_data', 'Exception in PetitionsSelectQueryMySQL::execute(): !e', array(
        '!e' => petitionslog_format_for_watchdog($e),
      ));
    }
    return $this;
  }

  /**
   * Builds the query.
   */
  protected function buildQuery() {
    $this->initializeQuery();
    $this->addArgumentsToQuery();
  }

  /**
   * Initializes the basic query.
   */
  protected function initializeQuery() {
    $this->query = new EntityFieldQueryExtraFields();
    $this->query
      ->entityCondition('entity_type', 'node')
      ->propertyCondition('type', array('petition'))
      ->addExtraField('', 'title', 'title', 'node')
      ->addExtraField('', 'created', 'created', 'node')
      ->addExtraField('', 'uid', 'uid', 'node');
  }

  /**
   * Adds the orderBy arguments to the query.
   */
  protected function addOrderByToQuery() {
    if (!$this->getOrderBy()) {
      return;
    }
    foreach ($this->getOrderBy() as $order_by) {
      $field = $order_by->field;
      $order_by_dir = $order_by->direction;
      if (!$this->isValidOrderByField($field)) {
        throw new Exception('addOrderByToQuery - Not a valid field: ' . $field);
      }
      if (!$this->isValidOrderByDirection($order_by_dir)) {
        throw new Exception('addOrderByToQuery - Not a valid direction: ' . $order_by_dir);
      }

      if (in_array($field, $this->getValidPetitionsOrderByFields())) {
        $mapped_field = $this->orderByFieldsMap[$field]['field'];
        $mapped_column = $this->orderByFieldsMap[$field]['column'];
        $mapped_dir = $this->orderByDirMap[$order_by_dir];

        $this->query->fieldCondition($mapped_field);
        $this->query->fieldOrderBy($mapped_field, $mapped_column, $mapped_dir);

        // Adding propertyOrderBy is necessary to ensure node table is joined.
        $this->query->propertyOrderBy('created', 'DESC');
      }
      else {
        $mapped_field = $this->orderByFieldsMap[$field]['field'];
        $mapped_dir = $this->orderByDirMap[$order_by_dir];
        $this->query->propertyOrderBy($mapped_field, $mapped_dir);
      }
    }
  }

  /**
   * Adds the supplied arguments to the query.
   */
  protected function addArgumentsToQuery() {
    // Start with the arguments that fail cheapest in case of bad input.
    $this->addUrlToQuery();

    $this->addBodyToQuery();
    $this->addCreatedArgumentsToQuery();
    $this->addStatusToQuery();
    $this->addPetitionIdsToQuery();
    $this->addResponseIdToQuery();
    $this->addSignatureCountArgumentsToQuery();
    $this->addSignatureThresholdArgumentsToQuery();
    $this->addTitleToQuery();
    $this->addIssuesToQuery();
    $this->addUidToQuery();

    // Clone the query before limiting its range.
    $this->countQuery = clone $this->query;

    // Add order by after clone, since order isn't necessary on count.
    $this->addOrderByToQuery();

    // This should be the only addition to the query after cloning it.
    $this->addOffsetAndLimitToQuery();
  }

  /**
   * Adds the url argument to the query.
   */
  protected function addUrlToQuery() {
    $url = $this->getURL();
    if ($url) {
      $nid = static::getPetitionIdFromUrl($url);
      if ($nid) {
        $this->query->propertyCondition('nid', $nid);
      }
      else {
        $path = petitions_data_get_path_from_full_url($url);
        $this->query->fieldCondition('field_legacy_path', 'value', $path);
      }
    }
  }

  /**
   * Adds the body argument to the query.
   */
  protected function addBodyToQuery() {
    if ($this->getBody()) {
      $this->query->fieldCondition('body', 'value', '%' . $this->getBody() . '%', 'LIKE');
    }
  }

  /**
   * Adds the created time-related arguments to the query.
   */
  protected function addCreatedArgumentsToQuery() {
    if ($this->getCreatedDate()) {
      $this->addCreatedAtToQuery();
    }
    else {
      $this->addCreatedAfterToQuery();
      $this->addCreatedBeforeToQuery();
    }
  }

  /**
   * Adds the createdAt argument to the query.
   */
  protected function addCreatedAtToQuery() {
    $this->query->fieldCondition('field_timestamp_published', 'value', $this->getCreatedDate());
  }

  /**
   * Adds the createdAfter argument to the query.
   */
  protected function addCreatedAfterToQuery() {
    if ($this->getStartDate()) {
      $this->query->fieldCondition('field_timestamp_published', 'value', $this->getStartDate(), '>');
    }
  }

  /**
   * Adds the createdBefore argument to the query.
   */
  protected function addCreatedBeforeToQuery() {
    if ($this->getEndDate()) {
      $this->query->fieldCondition('field_timestamp_published', 'value', $this->getEndDate(), '<');
    }
  }

  /**
   * Adds the offset and limit arguments to the query.
   */
  protected function addOffsetAndLimitToQuery() {
    $this->query->range($this->getOffset(), $this->getLimit());
  }

  /**
   * Adds the responseId argument to the query.
   */
  protected function addResponseIdToQuery() {
    if ($this->getResponseID()) {
      $this->query->fieldCondition('field_response_id', 'target_id', $this->getResponseID());
    }
  }

  /**
   * Adds the signature count-related arguments to the query.
   */
  protected function addSignatureCountArgumentsToQuery() {
    if ($this->getSignatureCount()) {
      $this->addSignatureCountToQuery();
    }
    else {
      $this->addSignatureCountCeilingToQuery();
      $this->addSignatureCountFloorToQuery();
    }
  }

  /**
   * Adds the signatureCount argument to the Query.
   */
  protected function addSignatureCountToQuery() {
    $this->query->fieldCondition('field_petition_signature_count', 'value', $this->getSignatureCount());
  }

  /**
   * Adds the signatureCountCeiling argument to the query.
   */
  protected function addSignatureCountCeilingToQuery() {
    if ($this->getSignatureCountCeiling()) {
      $this->query->fieldCondition('field_petition_signature_count', 'value', $this->getSignatureCountCeiling(), '<=');
    }
  }

  /**
   * Adds the signatureCountFloor argument to the query.
   */
  protected function addSignatureCountFloorToQuery() {
    if ($this->getSignatureCountFloor()) {
      $this->query->fieldCondition('field_petition_signature_count', 'value', $this->getSignatureCountFloor(), '>=');
    }
  }

  /**
   * Adds the signature threshold-related arguments to the query.
   */
  protected function addSignatureThresholdArgumentsToQuery() {
    if ($this->getSignatureThreshold()) {
      $this->addSignatureThresholdToQuery();
    }
    else {
      $this->addSignatureThresholdCeilingToQuery();
      $this->addSignatureThresholdFloorToQuery();
    }
  }

  /**
   * Adds the signatureThreshold argument to the Query.
   */
  protected function addSignatureThresholdToQuery() {
    $this->query->fieldCondition('field_petition_response_sign', 'value', $this->getSignatureThreshold());
  }

  /**
   * Adds the signatureThresholdCeiling argument to the query.
   */
  protected function addSignatureThresholdCeilingToQuery() {
    if ($this->getSignatureThresholdCeiling()) {
      $this->query->fieldCondition('field_petition_response_sign', 'value', $this->getSignatureThresholdCeiling(), '<=');
    }
  }

  /**
   * Adds the signatureThresholdFloor argument to the query.
   */
  protected function addSignatureThresholdFloorToQuery() {
    if ($this->getSignatureThresholdFloor()) {
      $this->query->fieldCondition('field_petition_response_sign', 'value', $this->getSignatureThresholdFloor(), '>=');
    }
  }

  /**
   * Adds the title argument to the query.
   */
  protected function addTitleToQuery() {
    if ($this->getTitle()) {
      $this->query->propertyCondition('title', '%' . $this->getTitle() . '%', 'LIKE');
    }
  }

  /**
   * Adds the issues argument to the query.
   */
  protected function addIssuesToQuery() {
    if ($this->getIssueIDs()) {
      $this->query->fieldCondition('field_petition_issues', 'tid', $this->getIssueIDs(), 'IN');
    }
  }

  /**
   * Adds petition IDs to the query.
   */
  protected function addPetitionIdsToQuery() {
    $petition_ids = $this->getPetitionIds();
    if (!empty($petition_ids)) {
      $nids = petition_extract_nids_from_petition_ids($petition_ids);
      if (!empty($nids)) {
        $this->query->propertyCondition('nid', $nids, 'IN');
      }

      $legacy_ids = petition_extract_legacy_ids_from_petition_ids($petition_ids);
      if (!empty($legacy_ids)) {
        $this->query->fieldCondition('field_legacy_id', 'value', $legacy_ids, 'IN');
      }

      // None of the given petition IDs were valid.
      if (empty($nids) && empty($legacy_ids)) {
        throw new PetitionNotFoundException();
      }
    }
  }

  /**
   * Adds User ID to the query.
   */
  protected function addUidToQuery() {
    $uid = $this->getUid();
    if (!empty($uid)) {
      $this->query->propertyCondition('uid', $uid);
    }
  }

  /**
   * Adds the status argument to the query.
   */
  protected function addStatusToQuery() {
    $include_statuses = $this->parseStatus();

    if ($include_statuses) {
      $this->query->fieldCondition('field_petition_status', 'value', $include_statuses, 'IN');
    }
    else {
      // There are no statuses so don't bother executing any query because there
      // aren't any results.
      throw new PetitionNotFoundException();
    }
  }

  /**
   * Gets a petition ID from a given petition URL.
   *
   * @param string $url
   *   The URL to get a petition ID from.
   *
   * @return mixed|false
   *   A petition ID corresponding to the given URL, or FALSE if there is no
   *   corresponding petition.
   */
  public static function getPetitionIdFromUrl($url) {
    $url_parts = parse_url($url);
    $path = substr($url_parts['path'], 1);
    $internal_path = drupal_get_normal_path($path);

    // Find a node ID, if present.
    $matches = array();
    preg_match('@^node/([1-9][0-9]*)$@', $internal_path, $matches);
    if (!isset($matches[1])) {
      return FALSE;
    }
    $nid = (int) $matches[1];

    return $nid;
  }

  /**
   * Executes the query.
   *
   * @return array
   *   An array of nodes with field data, or an empty array if there are no
   *   results.
   */
  protected function executeQuery() {
    $this->executeCountQuery();
    return $this->executeFullQuery();
  }

  /**
   * Executes the COUNT query to get the unlimited resultset size.
   */
  protected function executeCountQuery() {
    $this->resultsetCount = $this->countQuery->addTag('petitions_data_debug')->count()->execute();
  }

  /**
   * Executes the full query to get the resultset data.
   *
   * @return array
   *   An array of node data.
   */
  protected function executeFullQuery() {
    // If the COUNT query found no results, there's no reason to execute the
    // full query.
    if ($this->resultsetCount === 0) {
      return array();
    }

    $result = $this->query->addTag('petitions_data_debug')->execute();

    // Return early if no results.
    if (empty($result['node'])) {
      return array();
    }

    $nodes = $result['node'];
    field_attach_load('node', $nodes);
    return $nodes;
  }

  /**
   * Builds the results arrays from the query results.
   *
   * @param array $nodes
   *   An array of node data, as returned by
   *   PetitionsSelectQueryMysql::executeQuery.
   */
  protected function buildResults(array $nodes) {
    $result_objects = array();
    $result_arrays = array();
    foreach ($nodes as $node) {
      $response = isset($node->field_response_id[LANGUAGE_NONE][0]['target_id']) ? static::formatReturnResponse($node->field_response_id[LANGUAGE_NONE][0]['target_id']) : array();
      $response_status = isset($node->field_response_status[LANGUAGE_NONE][0]['value']) ? (int) $node->field_response_status[LANGUAGE_NONE][0]['value'] : NULL;

      // This is the pre-PHP 5.5.0 equivalent of array_column().
      // @see http://php.net/manual/en/function.array-column.php
      $issue_tids = array();
      if (!empty($node->field_petition_issues[LANGUAGE_NONE])) {
        $issue_tids = array_map(function ($element) {
          return $element['tid'];
        }, $node->field_petition_issues[LANGUAGE_NONE]);
      }

      $petition = new PetitionItem();
      // Make sure we have a legacy ID before we assign it.
      $legacy_id = (is_array($node->field_legacy_id) && isset($node->field_legacy_id[LANGUAGE_NONE])) ? (string) $node->field_legacy_id[LANGUAGE_NONE][0]['value'] : NULL;
      $timestamp_published = isset($node->field_timestamp_published[LANGUAGE_NONE][0]['value']) ? $node->field_timestamp_published[LANGUAGE_NONE][0]['value'] : NULL;
      $timestamp_reached_public = isset($node->field_timestamp_reached_public[LANGUAGE_NONE][0]['value']) ? $node->field_timestamp_reached_public[LANGUAGE_NONE][0]['value'] : NULL;
      $timestamp_reached_ready = isset($node->field_timestamp_reached_ready[LANGUAGE_NONE][0]['value']) ? $node->field_timestamp_reached_ready[LANGUAGE_NONE][0]['value'] : NULL;
      $timestamp_review_threshold_mail_sent = isset($node->field_review_threshold_mail_sent[LANGUAGE_NONE][0]['value']) ? $node->field_review_threshold_mail_sent[LANGUAGE_NONE][0]['value'] : NULL;
      $timestamp_responded = isset($node->field_timestamp_responded[LANGUAGE_NONE][0]['value']) ? $node->field_timestamp_responded[LANGUAGE_NONE][0]['value'] : NULL;
      $short_url = isset($node->field_short_url[LANGUAGE_NONE][0]['value']) ? $node->field_short_url[LANGUAGE_NONE][0]['value'] : NULL;
      $body = isset($node->body[LANGUAGE_NONE][0]['value']) ? $node->body[LANGUAGE_NONE][0]['value'] : '';

      $signature_count = db_select('signature_mail', 'sm')
        ->condition('petition_id', $node->nid)
        ->countQuery()
        ->execute()
        ->fetchField();
      @$node->signature_count = $signature_count;

      $petition
        ->setEntityId($node->nid)
        ->setLegacyId($legacy_id)
        ->setTitle($node->extraFields->title)
        ->setBody($body)
        ->setIssues($this->formatReturnIssues($issue_tids))

        // Since path aliases are cached, using url() for this value here will
        // ultimately be much more performant (not to mention less complex) than
        // adding a JOIN to the database query for it.
        ->setNiceUrl(drupal_get_path_alias("node/{$node->nid}"))

        ->setShortUrl($short_url)
        ->setReviewTimeframe($node->field_petition_review_timeframe[LANGUAGE_NONE][0]['value'])
        ->setSignatureCount((int) $node->signature_count)
        ->setSignatureThreshold((int) $node->field_petition_response_sign[LANGUAGE_NONE][0]['value'])
        ->setStatus($node->field_petition_status[LANGUAGE_NONE][0]['value'])
        ->setResponse($response)
        ->setCreated($node->extraFields->created)
        ->setPublished((int) $timestamp_published)
        ->setUid($node->extraFields->uid)
        ->setReachedPublic((int) $timestamp_reached_public)
        ->setReachedReady((int) $timestamp_reached_ready)
        ->setSignaturePublicThreshold((int) $node->field_petition_public_signatures[LANGUAGE_NONE][0]['value'])
        ->setResponseStatus($response_status)
        ->setReviewThresholdMailSent($timestamp_review_threshold_mail_sent)
        ->setClosed($timestamp_responded);
      if (!empty($node->field_legacy_path[LANGUAGE_NONE][0]['value'])) {
        $petition->setLegacyPath($node->field_legacy_path[LANGUAGE_NONE][0]['value']);
      }

      $result_objects[] = $petition;
      $result_arrays[] = $petition->toRestResponseItemArray();
    }

    $this->setResultObjects($result_objects);
    $this->setResult($result_arrays);
    $this->setCount($this->resultsetCount);
  }

  /**
   * Increment petition node metric count.
   *
   * @param int $petition_id
   *   Node petition ID
   *
   * @param string $petition_metric
   *   Petition Metric constant from PetitionItem class.
   *
   * @param bool $decrement
   *   If TRUE, will decrease metric count by one, otherwise increment.
   */
  public static function incrementPetitionMetricCount($petition_id, $petition_metric, $decrement = FALSE) {
    if (isset(self::$petitionMetricFieldsMap[$petition_metric])) {
      $nid = petition_get_nid($petition_id);
      if (!$nid) {
        watchdog('petitions_data', 'incrementPetitionMetricCount - Could not get petition nid for !id', array(
          '!id' => $petition_id,
        ));
        return;
      }

      $table_name = self::$petitionMetricFieldsMap[$petition_metric]['table'];
      $field_name = self::$petitionMetricFieldsMap[$petition_metric]['field'];
      $count_mod = $decrement ? ' - 1' : ' + 1';
      $result = db_update('field_data_' . $table_name)
        ->expression($field_name, $field_name . $count_mod)
        ->condition('entity_id', $nid)
        ->execute();

      if (!$result && !$decrement) {
        // If no rows updated, create a new row for the field with count = 1.
        $node = node_load($nid);
        if ($node) {
          $fields = array(
            'entity_type' => 'node',
            'bundle' => $node->type,
            'deleted' => 0,
            'entity_id' => $node->nid,
            'revision_id' => $node->vid,
            'language' => $node->language,
            'delta' => 0,
            $field_name => 1,
          );
          // DB insert used to skip apachesolr hooks to trigger node reindexing.
          db_insert('field_data_' . $table_name)->fields($fields)->execute();
          db_insert('field_revision_' . $table_name)->fields($fields)->execute();
        }
        else {
          watchdog('petitions_data', 'incrementPetitionMetricCount - Could not load petition_id !id', array(
            '!id' => $petition_id,
          ));
        }
      }
      elseif ($result) {
        // If node update was successful, update revision table as well.
        db_update('field_revision_' . $table_name)
          ->expression($field_name, $field_name . $count_mod)
          ->condition('entity_id', $nid)
          ->execute();
      }
    }
    else {
      watchdog('petitions_data', 'incrementPetitionMetricCount - invalid metric: !metric', array(
        '!metric' => $petition_metric,
      ));
    }
  }

  /**
   * Getter for petitionMetricFieldsMap.
   *
   * @return array
   *   Array of petition metric field mappings.
   */
  public static function getPetitionMetricFieldsMap() {
    return self::$petitionMetricFieldsMap;
  }

  /**
   * Bump a petition node's field value up or down by 1.
   *
   * Performs update without triggering node save hooks.
   *
   * @param int $petition_id
   *   Node petition ID
   * @param string $field_name
   *   Field name to bump, ie. "field_petition_last_alert".
   * @param string $column
   *   Column containing the value, ie. "value"
   * @param bool $decrement
   *   If TRUE, will decrement field otherwise increment by default.
   */
  public static function bumpPetitionField($petition_id, $field_name, $column, $decrement = FALSE) {
    $nid = petition_get_nid($petition_id);
    if (!$nid) {
      watchdog('petitions_data', 'bumpPetitionField - Could not get petition nid for !id', array(
        '!id' => $petition_id,
      ));
      return;
    }

    $value_column = $field_name . '_' . $column;
    $data_table = 'field_data_' . $field_name;
    $revision_table = 'field_revision_' . $field_name;
    $count_mod = $decrement ? ' - 1' : ' + 1';

    $result = db_update('field_data_' . $field_name)
      ->expression($value_column, $value_column . $count_mod)
      ->condition('entity_id', $nid)
      ->execute();

    if (!$result && !$decrement) {
      // If no rows updated, create a new row for the field with count = 1.
      $node = node_load($nid);
      if ($node) {
        $fields = array(
          'entity_type' => 'node',
          'bundle' => $node->type,
          'deleted' => 0,
          'entity_id' => $node->nid,
          'revision_id' => $node->vid,
          'language' => $node->language,
          'delta' => 0,
          $value_column => 1,
        );
        // DB insert used to skip apachesolr hooks to trigger node reindexing.
        db_insert($data_table)->fields($fields)->execute();
        db_insert($revision_table)->fields($fields)->execute();
      }
      else {
        watchdog('petitions_data', 'bumpPetitionField - Could not load petition_id !id', array(
          '!id' => $petition_id,
        ));
      }
    }
    elseif ($result) {
      // If node update was successful, update revision table as well.
      db_update($revision_table)
        ->expression($value_column, $value_column . $count_mod)
        ->condition('entity_id', $nid)
        ->execute();
    }
  }

  /**
   * Sets a petition's field to a value without triggering node save hooks.
   *
   * @param int $petition_id
   *   Node petition ID
   * @param string $field_name
   *   Field name to bump, ie. "field_petition_last_alert".
   * @param string $column
   *   Column containing the value, ie. "value"
   * @param string $value
   *   Value to set column to
   */
  public static function setPetitionFieldValue($petition_id, $field_name, $column, $value) {
    $nid = petition_get_nid($petition_id);
    if (!$nid) {
      watchdog('petitions_data', 'setPetitionFieldValue - Could not get petition nid for !id', array(
        '!id' => $petition_id,
      ));
      return;
    }

    $node = node_load($nid);
    $node->{$field_name}[LANGUAGE_NONE][0][$column] = $value;
    field_attach_update('node', $node);
    entity_get_controller('node')->resetCache(array($node->nid));
  }

}
