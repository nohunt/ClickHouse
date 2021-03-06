#!/usr/bin/env bash

set -e

CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. $CURDIR/../shell_config.sh

for i in `seq -w 0 2 20`; do
    $CLICKHOUSE_CLIENT -q "DROP TABLE IF EXISTS test.merge_item_$i"
    $CLICKHOUSE_CLIENT -q "CREATE TABLE test.merge_item_$i (d Int8) ENGINE = Memory"
    $CLICKHOUSE_CLIENT -q "INSERT INTO test.merge_item_$i VALUES ($i)"
done

$CLICKHOUSE_CLIENT -q "DROP TABLE IF EXISTS test.merge_storage"
$CLICKHOUSE_CLIENT -q "CREATE TABLE test.merge_storage (d Int8) ENGINE = Merge('test', '^merge_item_')"
$CLICKHOUSE_CLIENT --max_threads=1 -q "SELECT _table, d FROM test.merge_storage WHERE _table LIKE 'merge_item_1%' ORDER BY _table"
$CLICKHOUSE_CLIENT -q "DROP TABLE IF EXISTS test.merge_storage"

for i in `seq -w 0 2 20`; do $CLICKHOUSE_CLIENT -q "DROP TABLE IF EXISTS test.merge_item_$i"; done
