#include <stdlib.h>
#include <string.h>
#include <libxml/HTMLtree.h>
#include <libxml/HTMLparser.h>
#include "sanitize.h"

static unsigned move_children_before(xmlNodePtr element, xmlNodePtr before)
{
  xmlNodePtr item, next;
  unsigned count = 0;

  for (item = element->children; item; item = next)
    {
      next = item->next;
	  
      xmlUnlinkNode(item);
      xmlAddPrevSibling(before, item);

      ++count;
    }
  return count;
}

static void clean_element(xmlNodePtr element, struct sanitize_mode *mode)
{
  Dict *attributes;

  attributes = dict_get(mode->elements, (const char *)element->name);

  if (attributes)
    {
      /* element is allowed */

      xmlAttrPtr attr, next;
      ValueChecker *vc1, *vc2;
      xmlChar *value;
      for (attr = element->properties; attr; attr = next)
        {
          next = attr->next;
	  value = xmlNodeListGetString(element->doc, attr->children, 1);

	  vc1 = dict_get(attributes, (const char *)attr->name);
	  vc2 = dict_get(mode->common_attributes, (const char *)attr->name);

	  if (!(value_checker_check(vc1, (const char *)value) || value_checker_check(vc2, (const char *)value)))
	    {
	      xmlUnsetProp(element, attr->name);
	    }
	  
	  xmlFree(value);
        }
      return;
    }

  if (dict_get(mode->delete_elements, (const char *)element->name))
    {
      /* delete with children */
      xmlUnlinkNode(element);
      xmlFreeNode(element);

      return;
    }

  const char *rename_to = dict_get(mode->rename_elements, (const char *)element->name);

  if (!rename_to)
    {
      /* remove */
      move_children_before(element, element);
      xmlUnlinkNode(element);
      xmlFreeNode(element);

      return;
    }

  if (rename_to == Q_WHITESPACE)
    {
      xmlAddPrevSibling(element, xmlNewText(BAD_CAST(" ")));
      if (move_children_before(element, element))
        xmlAddPrevSibling(element, xmlNewText(BAD_CAST(" ")));
      
      xmlUnlinkNode(element);
      xmlFreeNode(element);

      return;
    }

  /* rename */
  xmlNodeSetName(element, BAD_CAST(rename_to));
  clean_element(element, mode); /* recursion */
}

static xmlNodePtr clean_node(xmlNodePtr node, struct sanitize_mode *mode)
{
  xmlNodePtr item, next;

  switch (node->type)
    {
    case XML_DOCUMENT_FRAG_NODE:
      for (item = node->children; item; )
        item = clean_node(item, mode);
      return node->next;
      
    case XML_TEXT_NODE:
      return node->next;

    case XML_CDATA_SECTION_NODE:
      next = node->next;
      {
        xmlChar *content = xmlNodeGetContent(node);
        xmlReplaceNode(node, xmlNewText(content ? content : BAD_CAST("")));
        xmlFreeNode(node);
        xmlFree(content);
      }
      return next;

    case XML_COMMENT_NODE:
      next = node->next;
      if (!mode->allow_comments)
        {
          xmlUnlinkNode(node);
          xmlFreeNode(node);
        }
      return next;

    case XML_ELEMENT_NODE:
      for (item = node->children; item; )
        item = clean_node(item, mode);
      next = node->next;
      clean_element(node, mode);
      return next;

    default:
      next = node->next;
      xmlUnlinkNode(node);
      xmlFreeNode(node);
      return next;
    }
}

static char *wrap_div(const char *s)
{
  size_t l = strlen(s);
  char *result = malloc(l + 12);
  strcpy(result, "<div>");
  strcpy(result+5, s);
  strcpy(result+5+l, "</div>");
  return result;
}

struct stream
{
  char *buffer;
  int length;
  int position;
};

static int stream_write_callback(void *context, const char *buffer, int len)
{
  struct stream *st = context;

  if (st->length - st->position < len)
    {
      st->length = (st->position + len + 4095) / 4096 * 4096;
      st->buffer = realloc(st->buffer, st->length);
    }
  memcpy(st->buffer + st->position, buffer, len);
  st->position += len;
  return len;
}

static void serialize_node(xmlNodePtr node, struct stream *stream)
{
  if (node->type == XML_DOCUMENT_FRAG_NODE)
    {
      for (node = node->children; node; node = node->next)
        serialize_node(node, stream);
    }
  else
    {
      xmlOutputBufferPtr buffer = xmlOutputBufferCreateIO(stream_write_callback,
                                                          NULL,
                                                          NULL,
                                                          xmlFindCharEncodingHandler("utf-8"));
      buffer->context = stream;
      htmlNodeDumpOutput(buffer, node->doc, node, "utf-8");
      xmlOutputBufferClose(buffer);
    }
}

static char *serialize_html(xmlNodePtr node)
{
  struct stream st;
  st.buffer = malloc(4096);
  st.length = 4096;
  st.position = 0;

  serialize_node(node, &st);

  st.buffer[st.position] = '\0';
  return st.buffer;
}

char *sanitize(const char *html, struct sanitize_mode *mode)
{
  char *wrapped = NULL;
  htmlDocPtr doc = NULL;
  xmlNodePtr entry = NULL;
  xmlNodePtr next = NULL;
  xmlNodePtr fragment = NULL;
  char *result = NULL;

  wrapped = wrap_div(html);
  doc = htmlReadDoc(BAD_CAST(wrapped), NULL, "utf-8",
		    HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
  if (!doc)
    goto err;
  
  entry = xmlDocGetRootElement(doc); /* html */
  if (!entry)
    goto err;
  
  entry = entry->children; /* body */
  if (!entry)
    goto err;

  entry = entry->children; /* div */
  if (!entry)
    goto err;

  entry = entry->children; /* div children */
  if (!entry)
    goto err;
  
  fragment = xmlNewDocFragment(doc);
  if (!fragment)
    goto err;

  for (; entry; entry = next)
    {
      next = entry->next;
      xmlUnlinkNode(entry);
      xmlAddChild(fragment, entry);
    }

  clean_node(fragment, mode);
  result = serialize_html(fragment);

 err:
  if (fragment)
    xmlFreeNode(fragment);
  if (doc)
    xmlFreeDoc(doc);
  if (wrapped)
    free(wrapped);

  return result;
}

