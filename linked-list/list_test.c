#include "sc_list.h"

#include <assert.h>

struct elem {
	int id;
	struct sc_list list;
};

static void test1(void)
{
	int k, i;
	struct elem a, b, c, d, e, *elem;
	struct sc_list list, *item, *tmp;

	sc_list_init(&a.list);
	sc_list_init(&b.list);
	sc_list_init(&c.list);
	sc_list_init(&d.list);
	sc_list_init(&e.list);

	a.id = 1;
	b.id = 2;
	c.id = 3;
	d.id = 4;
	e.id = 5;

	sc_list_init(&list);

	assert(sc_list_count(&list) == 0);
	tmp = sc_list_pop_head(&list);
	assert(tmp == NULL);

	tmp = sc_list_pop_tail(&list);
	assert(tmp == NULL);

	sc_list_add_tail(&list, &a.list);
	sc_list_add_tail(&list, &b.list);
	assert(sc_list_count(&list) == 2);
	tmp = sc_list_pop_tail(&list);
	elem = sc_list_entry(tmp, struct elem, list);
	assert(elem->id == b.id);

	sc_list_add_after(&list, &a.list, &b.list);
	assert(a.list.next == &b.list);

	sc_list_add_head(&list, &c.list);
	tmp = sc_list_pop_head(&list);
	elem = sc_list_entry(tmp, struct elem, list);
	assert(elem->id == c.id);

	sc_list_foreach (&list, item) {
		elem = sc_list_entry(item, struct elem, list);
	}

	sc_list_add_before(&list, &b.list, &e.list);
	assert(a.list.next == &e.list);

	sc_list_foreach (&list, item) {
		elem = sc_list_entry(item, struct elem, list);
	}

	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &d.list);

	sc_list_foreach (&list, item) {
		elem = sc_list_entry(item, struct elem, list);
	}

	sc_list_foreach (&list, item) {
		elem = sc_list_entry(item, struct elem, list);
	}

	sc_list_del(&list, &e.list);

	sc_list_foreach (&list, item) {
		elem = sc_list_entry(item, struct elem, list);
	}

	i = 1;
	sc_list_foreach (&list, item) {
		elem = sc_list_entry(item, struct elem, list);
		assert(elem->id == i);
		i++;
	}

	i = 4;
	sc_list_foreach_r(&list, item)
	{
		elem = sc_list_entry(item, struct elem, list);
		assert(elem->id == i);
		i--;
	}

	sc_list_clear(&list);

	assert(sc_list_is_empty(&list) == true);
	assert(sc_list_count(&list) == 0);
	assert(sc_list_head(&list) == NULL);
	assert(sc_list_tail(&list) == NULL);

	sc_list_add_tail(&list, &a.list);
	sc_list_add_tail(&list, &b.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &b.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_head(&list, &c.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_head(&list, &e.list);
	sc_list_add_tail(&list, &e.list);
	sc_list_add_head(&list, &e.list);
	sc_list_add_tail(&list, &e.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_tail(&list, &e.list);
	sc_list_add_head(&list, &e.list);
	sc_list_add_tail(&list, &e.list);

	assert(sc_list_head(&list) != NULL);
	assert(sc_list_tail(&list) != NULL);
	assert(sc_list_is_empty(&list) == false);
	assert(sc_list_count(&list) == 5);

	sc_list_clear(&list);

	sc_list_add_tail(&list, &a.list);
	sc_list_add_tail(&list, &b.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_tail(&list, &e.list);
	sc_list_add_tail(&list, &e.list);

	k = 0;
	sc_list_foreach_safe (&list, tmp, item) {
		if (k == 0) {
			sc_list_del(&list, &e.list);
		}

		elem = sc_list_entry(item, struct elem, list);

		k++;
		assert(elem->id == k);
	}

	sc_list_clear(&list);

	sc_list_add_tail(&list, &a.list);
	sc_list_add_tail(&list, &b.list);
	sc_list_add_tail(&list, &c.list);
	sc_list_add_tail(&list, &d.list);
	sc_list_add_tail(&list, &e.list);

	k = 6;
	sc_list_foreach_safe_r(&list, tmp, item)
	{
		if (k == 3) {
			sc_list_del(&list, &b.list);
			k--;
			continue;
		}

		elem = sc_list_entry(item, struct elem, list);

		k--;
		assert(elem->id == k);
	}
}

int main()
{
	test1();

	return 0;
}
