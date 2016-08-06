#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "mode.h"

const char *Q_WHITESPACE = NULL;

void mode_init_quarks(void)
{
  if (!Q_WHITESPACE)
    Q_WHITESPACE = quark("--whitespace--");
}

struct sanitize_mode *mode_new(void)
{
  struct sanitize_mode *mode;

  mode_init_quarks();

  mode = malloc(sizeof(struct sanitize_mode));
  mode->allow_comments = 0;
  mode->elements = dict_new((free_function_t)element_sanitizer_free);
  mode->delete_elements = dict_new(NULL);
  mode->rename_elements = dict_new((free_function_t)qfree);

  return mode;
}

void mode_free(struct sanitize_mode *mode)
{
  if (!mode)
    return;
  dict_free(mode->elements);
  dict_free(mode->delete_elements);
  dict_free(mode->rename_elements);
  free(mode);
}

static const char *get_attribute_quark(xmlNode *node, const char *attr_name, const char *default_value)
{
  xmlAttrPtr attr;

  for (attr = node->properties; attr; attr = attr->next)
    if (!xmlStrcmp(attr->name, BAD_CAST(attr_name)))
      {
        xmlChar* value;
        const char *result;

        value = xmlNodeListGetString(node->doc, attr->children, 1);
        if (*value)
          result = quark((const char *)value);
        else
          result = default_value;
        xmlFree(value);

        return result;
      }

  return default_value;
}

static void mode_load_attributes(ElementSanitizer *element_sanitizer, xmlNode *node)
{
  xmlAttrPtr attr;
  for (attr = node->properties; attr; attr = attr->next)
    {
      char *attribute;
      size_t name_len;
      int inverted = 0;
      xmlChar* re;

      attribute = strdup((const char *)attr->name);
      name_len = strlen((const char *)attr->name);
      if (name_len >= 4 && !strcmp((const char *)attr->name + name_len - 4, ".not"))
        {
          attribute[name_len - 4] = '\0';
          inverted = 1;
        }

      re = xmlNodeListGetString(node->doc, attr->children, 1);
      element_sanitizer_add_regex(element_sanitizer, attribute, (const char *)re, inverted);
      xmlFree(re);
      free(attribute);
    }
}

static struct sanitize_mode *mode_deserialize(xmlDocPtr doc)
{
  xmlNode *root_element, *node, *child;
  xmlAttrPtr attr;
  struct sanitize_mode *mode;

  root_element = xmlDocGetRootElement(doc);
  if (strcmp((const char *)root_element->name, "mode"))
    return NULL;

  mode = mode_new();

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
          for (child = node->children; child; child = child->next)
            if (child->type == XML_ELEMENT_NODE)
              {
                ElementSanitizer *element_sanitizer = element_sanitizer_new();
                mode_load_attributes(element_sanitizer, node);  /* common attributes */
                mode_load_attributes(element_sanitizer, child);
                dict_replace(mode->elements, (const char *)child->name, element_sanitizer);
              }
        }
      else if (!xmlStrcmp(node->name, BAD_CAST("rename")))
        {
          const char *to = get_attribute_quark(node, "to", Q_WHITESPACE);

          for (child = node->children; child; child = child->next)
            if (child->type == XML_ELEMENT_NODE)
              {
                dict_replace(mode->rename_elements, (const char *)child->name, (char *)to);
              }
        }
      else if (!xmlStrcmp(node->name, BAD_CAST("delete")))
        {
          for (child = node->children; child; child = child->next)
            if (child->type == XML_ELEMENT_NODE)
              dict_replace(mode->delete_elements, (const char *)child->name, (char *)Q_WHITESPACE);
        }
    }

  return mode;
}

struct sanitize_mode *mode_load(const char *filename)
{
  xmlDocPtr doc = xmlReadFile(filename, NULL, 0);
  if (doc == NULL)
    return NULL;
  struct sanitize_mode *mode = mode_deserialize(doc);
  xmlFreeDoc(doc);
  return mode;
}

struct sanitize_mode *mode_memory(const char *data)
{
  xmlDocPtr doc = xmlReadDoc(BAD_CAST(data), NULL, NULL, 0);
  if (doc == NULL)
    return NULL;
  struct sanitize_mode *mode = mode_deserialize(doc);
  xmlFreeDoc(doc);
  return mode;
}

