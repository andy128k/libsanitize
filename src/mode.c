#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "mode.h"

struct attribute *attribute_new(const char *name, const char *value)
{
  struct attribute *attr;
  size_t name_len;
  
  attr = malloc(sizeof(struct attribute));
  name_len = strlen(name);
  if (name_len < 4 || strcmp(name + name_len - 4, ".not"))
    {
      attr->name = strdup(name);
      attr->inverted = 0;
    }
  else
    {
      attr->name = strndup(name, name_len - 4);
      attr->inverted = 1;
    }

  if (value && *value)
    {
      attr->value = strdup(value);
      regcomp(&attr->preg, attr->value, REG_EXTENDED | REG_ICASE | REG_NOSUB);
    }
  else
    {
      attr->value = NULL;
    }
  return attr;
}

void attribute_free(struct attribute *attr)
{
  if (!attr)
    return;
  free(attr->name);
  if (attr->value)
    {
      free(attr->value);
      regfree(&attr->preg);
    }
  free(attr);
}

int attribute_check(struct attribute *attr, const char *name, const char *value)
{
  if (strcmp(attr->name, name))
    return 0;

  if (!attr->value)
    return 1;

  int r = !regexec(&attr->preg, value, 0, NULL, 0);
  if (attr->inverted)
    r = !r;
  return r;
}

int regexec(const regex_t *preg, const char *string, size_t nmatch,
            regmatch_t pmatch[], int eflags);


struct element *element_new(const char *tagname)
{
  struct element *el = malloc(sizeof(struct element));
  el->tagname = strdup(tagname);
  el->attributes = array_new((array_item_free_t)attribute_free);
  return el;
}

void element_free(struct element *el)
{
  if (!el)
    return;
  free(el->tagname);
  array_free(el->attributes);
  free(el);
}

struct sanitize_mode *mode_new(int allow_comments,
			       Array *elements,
			       Array *common_attributes,
			       Array *whitespace_elements)
{
  struct sanitize_mode *mode;

  mode = malloc(sizeof(struct sanitize_mode));

  mode->allow_comments = allow_comments;
  if (elements)
    mode->elements = elements;
  else
    mode->elements = array_new((array_item_free_t)element_free);

  if (common_attributes)
    mode->common_attributes = elements;
  else
    mode->common_attributes = array_new((array_item_free_t)attribute_free);

  if (whitespace_elements)
    mode->whitespace_elements = whitespace_elements;
  else
    mode->whitespace_elements = array_new((array_item_free_t)free);

  return mode;
}

void mode_free(struct sanitize_mode *mode)
{
  if (!mode)
    return;
  if (mode->elements)
    array_free(mode->elements);
  if (mode->common_attributes)
    array_free(mode->common_attributes);
  if (mode->whitespace_elements)
    array_free(mode->whitespace_elements);
  free(mode);
}

static int element_has_tagname(struct element *el, const char *tagname)
{
  return 0 == strcmp(el->tagname, tagname);
}

struct element *mode_find_element(struct sanitize_mode *mode, const char *tagname)
{
  return array_find(mode->elements, (array_item_predicate_t)element_has_tagname, tagname);
}

static void mode_load_attributes(Array *attributes, xmlNode *node)
{
  xmlAttrPtr attr;
  for (attr = node->properties; attr; attr = attr->next)
    {
      xmlChar* value = xmlNodeListGetString(node->doc, attr->children, 1);
      array_append(attributes,
		   attribute_new((const char *)attr->name,
				 (const char *)value));
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
		struct element *el = element_new((const char *)child->name);
		mode_load_attributes(el->attributes, child);
		array_append(mode->elements, el);
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


