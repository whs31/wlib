After relevant source changes, run `wkit fmt` (the canonical formatter) and then `wkit check` (formatting and clang-tidy audit). Fix every finding the check reports in code you touched before considering the change done.

Never commit or push unless explicitly requested.
