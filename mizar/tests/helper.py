import logging

logging.basicConfig()
logger = logging.getLogger()
logger.setLevel(logging.INFO)


def do_add_remove_maglev(test, table, add, remove, exp_table, exp_prev_ele_map):
    logger.info("===Add/Remove Maglev test===")
    if add:
        logger.info("Adding {} to the table.".format(add))
        table.add(add)
    if remove:
        logger.info("Removing {} from the table.".format(remove))
        table.remove(remove)
    act_table = table.get_table()
    act_prev_ele_map = table.get_prev_elements_map()
    test.assertEqual(exp_table, act_table)
    test.assertEqual(act_prev_ele_map, exp_prev_ele_map)
    logger.info("Number of elements replaced {}".format(
        table.elements_replaced))
    logger.info("Number of elements replacing {}".format(
        table.elements_replacing))
