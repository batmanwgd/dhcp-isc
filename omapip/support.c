/* support.c

   Subroutines providing general support for objects. */

/*
 * Copyright (c) 1996-1999 Internet Software Consortium.
 * Use is subject to license terms which appear in the file named
 * ISC-LICENSE that should have accompanied this file when you
 * received it.   If a file named ISC-LICENSE did not accompany this
 * file, or you are not sure the one you have is correct, you may
 * obtain an applicable copy of the license at:
 *
 *             http://www.isc.org/isc-license-1.0.html. 
 *
 * This file is part of the ISC DHCP distribution.   The documentation
 * associated with this file is listed in the file DOCUMENTATION,
 * included in the top-level directory of this release.
 *
 * Support and other services are available for ISC products - see
 * http://www.isc.org for more information.
 */

#include <omapip/omapip.h>

omapi_object_type_t *omapi_type_connection;
omapi_object_type_t *omapi_type_listener;
omapi_object_type_t *omapi_type_io_object;
omapi_object_type_t *omapi_type_datagram;
omapi_object_type_t *omapi_type_generic;
omapi_object_type_t *omapi_type_protocol;
omapi_object_type_t *omapi_type_protocol_listener;
omapi_object_type_t *omapi_type_waiter;
omapi_object_type_t *omapi_type_remote;
omapi_object_type_t *omapi_type_message;

omapi_object_type_t *omapi_object_types;
int omapi_object_type_count;
static int ot_max;

isc_result_t omapi_init (void)
{
	isc_result_t status;

	/* Register all the standard object types... */
	status = omapi_object_type_register (&omapi_type_connection,
					     "connection",
					     omapi_connection_set_value,
					     omapi_connection_get_value,
					     omapi_connection_destroy,
					     omapi_connection_signal_handler,
					     omapi_connection_stuff_values,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_listener,
					     "listener",
					     omapi_listener_set_value,
					     omapi_listener_get_value,
					     omapi_listener_destroy,
					     omapi_listener_signal_handler,
					     omapi_listener_stuff_values,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_io_object,
					     "io",
					     omapi_io_set_value,
					     omapi_io_get_value,
					     omapi_io_destroy,
					     omapi_io_signal_handler,
					     omapi_io_stuff_values,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_generic,
					     "generic",
					     omapi_generic_set_value,
					     omapi_generic_get_value,
					     omapi_generic_destroy,
					     omapi_generic_signal_handler,
					     omapi_generic_stuff_values,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_protocol,
					     "protocol",
					     omapi_protocol_set_value,
					     omapi_protocol_get_value,
					     omapi_protocol_destroy,
					     omapi_protocol_signal_handler,
					     omapi_protocol_stuff_values,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_protocol_listener,
					     "protocol-listener",
					     omapi_protocol_listener_set_value,
					     omapi_protocol_listener_get_value,
					     omapi_protocol_listener_destroy,
					     omapi_protocol_listener_signal,
					     omapi_protocol_listener_stuff,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_message,
					     "message",
					     omapi_message_set_value,
					     omapi_message_get_value,
					     omapi_message_destroy,
					     omapi_message_signal_handler,
					     omapi_message_stuff_values,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_object_type_register (&omapi_type_waiter,
					     "waiter",
					     0,
					     0,
					     0,
					     omapi_waiter_signal_handler, 0,
					     0, 0, 0);
	if (status != ISC_R_SUCCESS)
		return status;

	/* This seems silly, but leave it. */
	return ISC_R_SUCCESS;
}

isc_result_t omapi_object_type_register (omapi_object_type_t **type,
					 char *name,
					 isc_result_t (*set_value)
						 (omapi_object_t *,
						  omapi_object_t *,
						  omapi_data_string_t *,
						  omapi_typed_data_t *),
					 isc_result_t (*get_value)
						(omapi_object_t *,
						 omapi_object_t *,
						 omapi_data_string_t *,
						 omapi_value_t **),
					 isc_result_t (*destroy)
						(omapi_object_t *, char *),
					 isc_result_t (*signal_handler)
						 (omapi_object_t *,
						  char *, va_list),
					 isc_result_t (*stuff_values)
						(omapi_object_t *,
						 omapi_object_t *,
						 omapi_object_t *),
					 isc_result_t (*lookup)
						(omapi_object_t **,
						 omapi_object_t *,
						 omapi_object_t *),
					 isc_result_t (*create)
						(omapi_object_t **,
						 omapi_object_t *),
					 isc_result_t (*delete)
						(omapi_object_t *,
						 omapi_object_t *))
{
	omapi_object_type_t *t;

	t = malloc (sizeof *t);
	if (!t)
		return ISC_R_NOMEMORY;
	memset (t, 0, sizeof *t);

	t -> name = name;
	t -> set_value = set_value;
	t -> get_value = get_value;
	t -> destroy = destroy;
	t -> signal_handler = signal_handler;
	t -> stuff_values = stuff_values;
	t -> lookup = lookup;
	t -> create = create;
	t -> delete = delete;
	t -> next = omapi_object_types;
	omapi_object_types = t;
	if (type)
		*type = t;
	return ISC_R_SUCCESS;
}

isc_result_t omapi_signal (omapi_object_t *handle, char *name, ...)
{
	va_list ap;
	omapi_object_t *outer;
	isc_result_t status;

	va_start (ap, name);
	for (outer = handle; outer -> outer; outer = outer -> outer)
		;
	if (outer -> type -> signal_handler)
		status = (*(outer -> type -> signal_handler)) (outer,
							       name, ap);
	else
		status = ISC_R_NOTFOUND;
	va_end (ap);
	return status;
}

isc_result_t omapi_signal_in (omapi_object_t *handle, char *name, ...)
{
	va_list ap;
	omapi_object_t *outer;
	isc_result_t status;

	if (!handle)
		return ISC_R_NOTFOUND;
	va_start (ap, name);

	if (handle -> type -> signal_handler)
		status = (*(handle -> type -> signal_handler)) (handle,
								name, ap);
	else
		status = ISC_R_NOTFOUND;
	va_end (ap);
	return status;
}

isc_result_t omapi_set_value (omapi_object_t *h,
			      omapi_object_t *id,
			      omapi_data_string_t *name,
			      omapi_typed_data_t *value)
{
	omapi_object_t *outer;

	for (outer = h; outer -> outer; outer = outer -> outer)
		;
	if (outer -> type -> set_value)
		return (*(outer -> type -> set_value)) (outer,
							id, name, value);
	return ISC_R_NOTFOUND;
}

isc_result_t omapi_set_value_str (omapi_object_t *h,
				  omapi_object_t *id,
				  char *name,
				  omapi_typed_data_t *value)
{
	omapi_object_t *outer;
	omapi_data_string_t *nds;
	isc_result_t status;

	nds = (omapi_data_string_t *)0;
	status = omapi_data_string_new (&nds, strlen (name),
					"omapi_set_value_str");
	if (status != ISC_R_SUCCESS)
		return status;
	memcpy (nds -> value, name, strlen (name));

	return omapi_set_value (h, id, nds, value);
}

isc_result_t omapi_set_boolean_value (omapi_object_t *h, omapi_object_t *id,
				      char *name, int value)
{
	isc_result_t status;
	omapi_typed_data_t *tv = (omapi_typed_data_t *)0;
	omapi_data_string_t *n = (omapi_data_string_t *)0;
	int len;
	int ip;

	status = omapi_data_string_new (&n, strlen (name),
					"omapi_set_boolean_value");
	if (status != ISC_R_SUCCESS)
		return status;
	memcpy (n -> value, name, strlen (name));

	status = omapi_typed_data_new (&tv, omapi_datatype_int, value);
	if (status != ISC_R_SUCCESS) {
		omapi_data_string_dereference (&n,
					       "omapi_set_boolean_value");
		return status;
	}

	status = omapi_set_value (h, id, n, tv);
	omapi_data_string_dereference (&n, "omapi_set_boolean_value");
	omapi_typed_data_dereference (&tv, "omapi_set_boolean_value");
	return status;
}

isc_result_t omapi_set_int_value (omapi_object_t *h, omapi_object_t *id,
				  char *name, int value)
{
	isc_result_t status;
	omapi_typed_data_t *tv = (omapi_typed_data_t *)0;
	omapi_data_string_t *n = (omapi_data_string_t *)0;
	int len;
	int ip;

	status = omapi_data_string_new (&n, strlen (name),
					"omapi_set_int_value");
	if (status != ISC_R_SUCCESS)
		return status;
	memcpy (n -> value, name, strlen (name));

	status = omapi_typed_data_new (&tv, omapi_datatype_int, value);
	if (status != ISC_R_SUCCESS) {
		omapi_data_string_dereference (&n,
					       "omapi_set_int_value");
		return status;
	}

	status = omapi_set_value (h, id, n, tv);
	omapi_data_string_dereference (&n, "omapi_set_int_value");
	omapi_typed_data_dereference (&tv, "omapi_set_int_value");
	return status;
}

isc_result_t omapi_set_object_value (omapi_object_t *h, omapi_object_t *id,
				     char *name, omapi_object_t *value)
{
	isc_result_t status;
	omapi_typed_data_t *tv = (omapi_typed_data_t *)0;
	omapi_data_string_t *n = (omapi_data_string_t *)0;
	int len;
	int ip;

	status = omapi_data_string_new (&n, strlen (name),
					"omapi_set_object_value");
	if (status != ISC_R_SUCCESS)
		return status;
	memcpy (n -> value, name, strlen (name));

	status = omapi_typed_data_new (&tv, omapi_datatype_object, value);
	if (status != ISC_R_SUCCESS) {
		omapi_data_string_dereference (&n,
					       "omapi_set_object_value");
		return status;
	}

	status = omapi_set_value (h, id, n, tv);
	omapi_data_string_dereference (&n, "omapi_set_object_value");
	omapi_typed_data_dereference (&tv, "omapi_set_object_value");
	return status;
}

isc_result_t omapi_set_string_value (omapi_object_t *h, omapi_object_t *id,
				     char *name, char *value)
{
	isc_result_t status;
	omapi_typed_data_t *tv = (omapi_typed_data_t *)0;
	omapi_data_string_t *n = (omapi_data_string_t *)0;
	int len;
	int ip;

	status = omapi_data_string_new (&n, strlen (name),
					"omapi_set_string_value");
	if (status != ISC_R_SUCCESS)
		return status;
	memcpy (n -> value, name, strlen (name));

	status = omapi_typed_data_new (&tv, omapi_datatype_string, value);
	if (status != ISC_R_SUCCESS) {
		omapi_data_string_dereference (&n,
					       "omapi_set_string_value");
		return status;
	}

	status = omapi_set_value (h, id, n, tv);
	omapi_data_string_dereference (&n, "omapi_set_string_value");
	omapi_typed_data_dereference (&tv, "omapi_set_string_value");
	return status;
}

isc_result_t omapi_get_value (omapi_object_t *h,
			      omapi_object_t *id,
			      omapi_data_string_t *name,
			      omapi_value_t **value)
{
	omapi_object_t *outer;

	for (outer = h; outer -> outer; outer = outer -> outer)
		;
	if (outer -> type -> get_value)
		return (*(outer -> type -> get_value)) (outer,
							id, name, value);
	return ISC_R_NOTFOUND;
}

isc_result_t omapi_get_value_str (omapi_object_t *h,
				  omapi_object_t *id,
				  char *name,
				  omapi_value_t **value)
{
	omapi_object_t *outer;
	omapi_data_string_t *nds;
	isc_result_t status;

	nds = (omapi_data_string_t *)0;
	status = omapi_data_string_new (&nds, strlen (name),
					"omapi_get_value_str");
	if (status != ISC_R_SUCCESS)
		return status;
	memcpy (nds -> value, name, strlen (name));

	for (outer = h; outer -> outer; outer = outer -> outer)
		;
	if (outer -> type -> get_value)
		return (*(outer -> type -> get_value)) (outer,
							id, nds, value);
	return ISC_R_NOTFOUND;
}

isc_result_t omapi_stuff_values (omapi_object_t *c,
				 omapi_object_t *id,
				 omapi_object_t *o)
{
	omapi_object_t *outer;

	for (outer = o; outer -> outer; outer = outer -> outer)
		;
	if (outer -> type -> stuff_values)
		return (*(outer -> type -> stuff_values)) (c, id, outer);
	return ISC_R_NOTFOUND;
}

isc_result_t omapi_object_create (omapi_object_t **obj, omapi_object_t *id,
				  omapi_object_type_t *type)
{
	if (!type -> create)
		return ISC_R_NOTIMPLEMENTED;
	return (*(type -> create)) (obj, id);
}

isc_result_t omapi_object_update (omapi_object_t *obj, omapi_object_t *id,
				  omapi_object_t *src, omapi_handle_t handle)
{
	omapi_generic_object_t *gsrc;
	isc_result_t status;
	int i;

	if (!src)
		return ISC_R_INVALIDARG;
	if (src -> type != omapi_type_generic)
		return ISC_R_NOTIMPLEMENTED;
	gsrc = (omapi_generic_object_t *)src;
	for (i = 0; i < gsrc -> nvalues; i++) {
		status = omapi_set_value (obj, id,
					  gsrc -> values [i] -> name,
					  gsrc -> values [i] -> value);
		if (status != ISC_R_SUCCESS)
			return status;
	}
	if (handle)
		omapi_set_int_value (obj, id, "remote-handle", handle);
	omapi_signal (obj, "updated");
	return ISC_R_SUCCESS;
}

int omapi_data_string_cmp (omapi_data_string_t *s1, omapi_data_string_t *s2)
{
	int len;
	int rv;

	if (s1 -> len > s2 -> len)
		len = s2 -> len;
	else
		len = s1 -> len;
	rv = memcmp (s1 -> value, s2 -> value, len);
	if (rv)
		return rv;
	if (s1 -> len > s2 -> len)
		return 1;
	else if (s1 -> len < s2 -> len)
		return -1;
	return 0;
}

int omapi_ds_strcmp (omapi_data_string_t *s1, char *s2)
{
	int len, slen;
	int rv;

	slen = strlen (s2);
	if (slen > s1 -> len)
		len = s1 -> len;
	else
		len = slen;
	rv = memcmp (s1 -> value, s2, len);
	if (rv)
		return rv;
	if (s1 -> len > slen)
		return 1;
	else if (s1 -> len < slen)
		return -1;
	return 0;
}

int omapi_td_strcmp (omapi_typed_data_t *s1, char *s2)
{
	int len, slen;
	int rv;

	/* If the data type is not compatible, never equal. */
	if (s1 -> type != omapi_datatype_data &&
	    s1 -> type != omapi_datatype_string)
		return -1;

	slen = strlen (s2);
	if (slen > s1 -> u.buffer.len)
		len = s1 -> u.buffer.len;
	else
		len = slen;
	rv = memcmp (s1 -> u.buffer.value, s2, len);
	if (rv)
		return rv;
	if (s1 -> u.buffer.len > slen)
		return 1;
	else if (s1 -> u.buffer.len < slen)
		return -1;
	return 0;
}

isc_result_t omapi_make_value (omapi_value_t **vp, omapi_data_string_t *name,
			       omapi_typed_data_t *value, char *caller)
{
	isc_result_t status;

	status = omapi_value_new (vp, caller);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_data_string_reference (&(*vp) -> name, name, caller);
	if (status != ISC_R_SUCCESS) {
		omapi_value_dereference (vp, caller);
		return status;
	}
	if (value) {
		status = omapi_typed_data_reference (&(*vp) -> value,
						     value, caller);
		if (status != ISC_R_SUCCESS) {
			omapi_value_dereference (vp, caller);
			return status;
		}
	}
	return ISC_R_SUCCESS;
}

isc_result_t omapi_make_const_value (omapi_value_t **vp,
				     omapi_data_string_t *name,
				     u_int8_t *value, int len, char *caller)
{
	isc_result_t status;

	status = omapi_value_new (vp, caller);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_data_string_reference (&(*vp) -> name, name, caller);
	if (status != ISC_R_SUCCESS) {
		omapi_value_dereference (vp, caller);
		return status;
	}
	if (value) {
		status = omapi_typed_data_new (&(*vp) -> value,
					       omapi_datatype_data, len);
		if (status != ISC_R_SUCCESS) {
			omapi_value_dereference (vp, caller);
			return status;
		}
		memcpy ((*vp) -> value -> u.buffer.value, value, len);
	}
	return ISC_R_SUCCESS;
}

isc_result_t omapi_make_int_value (omapi_value_t **vp,
				   omapi_data_string_t *name,
				   int value, char *caller)
{
	isc_result_t status;

	status = omapi_value_new (vp, caller);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_data_string_reference (&(*vp) -> name, name, caller);
	if (status != ISC_R_SUCCESS) {
		omapi_value_dereference (vp, caller);
		return status;
	}
	if (value) {
		status = omapi_typed_data_new (&(*vp) -> value,
					       omapi_datatype_int);
		if (status != ISC_R_SUCCESS) {
			omapi_value_dereference (vp, caller);
			return status;
		}
		(*vp) -> value -> u.integer = value;
	}
	return ISC_R_SUCCESS;
}

isc_result_t omapi_make_handle_value (omapi_value_t **vp,
				      omapi_data_string_t *name,
				      omapi_object_t *value, char *caller)
{
	isc_result_t status;

	status = omapi_value_new (vp, caller);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_data_string_reference (&(*vp) -> name, name, caller);
	if (status != ISC_R_SUCCESS) {
		omapi_value_dereference (vp, caller);
		return status;
	}
	if (value) {
		status = omapi_typed_data_new (&(*vp) -> value,
					       omapi_datatype_int);
		if (status != ISC_R_SUCCESS) {
			omapi_value_dereference (vp, caller);
			return status;
		}
		status = (omapi_object_handle
			  ((omapi_handle_t *)&(*vp) -> value -> u.integer,
			   value));
		if (status != ISC_R_SUCCESS) {
			omapi_value_dereference (vp, caller);
			return status;
		}
	}
	return ISC_R_SUCCESS;
}

isc_result_t omapi_make_string_value (omapi_value_t **vp,
				      omapi_data_string_t *name,
				      char *value, char *caller)
{
	isc_result_t status;

	status = omapi_value_new (vp, caller);
	if (status != ISC_R_SUCCESS)
		return status;

	status = omapi_data_string_reference (&(*vp) -> name, name, caller);
	if (status != ISC_R_SUCCESS) {
		omapi_value_dereference (vp, caller);
		return status;
	}
	if (value) {
		status = omapi_typed_data_new (&(*vp) -> value,
					       omapi_datatype_string, value);
		if (status != ISC_R_SUCCESS) {
			omapi_value_dereference (vp, caller);
			return status;
		}
	}
	return ISC_R_SUCCESS;
}

isc_result_t omapi_get_int_value (u_int32_t *v, omapi_typed_data_t *t)
{
	u_int32_t rv;

	if (t -> type == omapi_datatype_int) {
		*v = t -> u.integer;
		return ISC_R_SUCCESS;
	} else if (t -> type == omapi_datatype_string ||
		   t -> type == omapi_datatype_data) {
		if (t -> u.buffer.len != sizeof (rv))
			return ISC_R_INVALIDARG;
		memcpy (&rv, t -> u.buffer.value, sizeof rv);
		*v = ntohl (rv);
		return ISC_R_SUCCESS;
	}
	return ISC_R_INVALIDARG;
}
