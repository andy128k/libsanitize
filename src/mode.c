#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "mode.h"

struct sanitize_mode *mode_new(int allow_comments,
			       Dict *elements,
			       Dict *common_attributes,
			       Array *whitespace_elements)
{
  struct sanitize_mode *mode;

  mode = malloc(sizeof(struct sanitize_mode));

  mode->allow_comments = allow_comments;
  if (elements)
    mode->elements = elements;
  else
    mode->elements = dict_new((free_function_t)dict_free);

  if (common_attributes)
    mode->common_attributes = common_attributes;
  else
    mode->common_attributes = dict_new((free_function_t)value_checker_free);

  if (whitespace_elements)
    mode->whitespace_elements = whitespace_elements;
  else
    mode->whitespace_elements = array_new((free_function_t)free);

  return mode;
}

void mode_free(struct sanitize_mode *mode)
{
  if (!mode)
    return;
  if (mode->elements)
    dict_free(mode->elements);
  if (mode->common_attributes)
    dict_free(mode->common_attributes);
  if (mode->whitespace_elements)
    array_free(mode->whitespace_elements);
  free(mode);
}

static void mode_load_attributes(Dict *attributes, xmlNode *node)
{
  xmlAttrPtr attr;
  for (attr = node->properties; attr; attr = attr->next)
    {
      size_t name_len;
      int inverted = 0;
      ValueChecker *vc;
      xmlChar* value;

      name_len = strlen((const char *)attr->name);
      if (name_len >= 4 && !strcmp((const char *)attr->name + name_len - 4, ".not"))
	{
	  name_len -= 4;
	  inverted = 1;
	}

      vc = dict_getn(attributes, (const char *)attr->name, name_len);
      if (!vc)
	{
	  vc = value_checker_new();
	  dict_replacen(attributes, (const char *)attr->name, name_len, vc);
	}

      value = xmlNodeListGetString(node->doc, attr->children, 1);
      value_checker_add_regex(vc, (const char *)value, inverted);
      xmlFree(value);
    }
}

struct sanitize_mode *mode_load(const char *filename)
{
  xmlDocPtr doc;
  xmlNode *root_element, *node, *child;
  xmlAttrPtr attr;
  struct sanitize_mode *mode;
  
  doc = xmlReadFile(filename, NULL, 0);
  if (doc == NULL)
    return NULL;
  
  root_element = xmlDocGetRootElement(doc);
  if (strcmp((const char *)root_element->name, "mode"))
    {
      xmlFreeDoc(doc);
      return NULL;
    }

  mode = mode_new(0, NULL, NULL, NULL);

  for (attr = root_element->properties; attr; attr = attr->next)
    if (!xmlStrcmp(attr->name, BAD_CAST("allow_comments")) ||
	!xmlStrcmp(attr->name, BAD_CAST("allow-comments")))
      {
	xmlChar* value = xmlNodeListGetString(doc, attr->children, 1);
	mode->allow_comments =
	  !xmlStrcasecmp(value, BAD_CAST("1"))    ||
	  !xmlStrcasecmp(value, BAD_CAST("yes"))  ||
	  !xmlStrcasecmp(value, BAD_CAST("y"))    ||
	  !xmlStrcasecmp(value, BAD_CAST("true")) ||
	  !xmlStrcasecmp(value, BAD_CAST("t"))    ||
	  !xmlStrcasecmp(value, BAD_CAST("on"));
	xmlFree(value); 
      }
  
  for (node = root_element->children; node; node = node->next)
    {
      if (node->type != XML_ELEMENT_NODE)
	continue;

      if (!xmlStrcmp(node->name, BAD_CAST("elements")))
	{
	  mode_load_attributes(mode->common_attributes, node);

	  for (child = node->children; child; child = child->next)
	    if (child->type == XML_ELEMENT_NODE)
	      {
		Dict *attributes = dict_new((free_function_t)value_checker_free);
		mode_load_attributes(attributes, child);
		dict_replace(mode->elements, (const char *)child->name, attributes);
	      }
	}
      else if (!xmlStrcmp(node->name, BAD_CAST("whitespace_elements")) ||
	       !xmlStrcmp(node->name, BAD_CAST("whitespace-elements")))
	{
	  for (child = node->children; child; child = child->next)
	    if (child->type == XML_ELEMENT_NODE)
	      array_append(mode->whitespace_elements, strdup((const char *)child->name));
	}
    }
  
  xmlFreeDoc(doc);
  return mode;
}


