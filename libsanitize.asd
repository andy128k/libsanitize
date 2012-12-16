(in-package :asdf)

(defsystem "libsanitize"
    :depends-on (:cffi :trivial-garbage)
    :components ((:file "libsanitize")))

