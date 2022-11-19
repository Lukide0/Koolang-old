doctest_dep = dependency('doctest', required : false)

if not doctest_dep.found()
    doctest_proj = subproject('doctest')
    doctest_dep = doctest_proj.get_variable('doctest_dep')
endif

test_sources = {
    'Tokenizer': {
        'libs': [ tokenizer_lib, term_lib ],
        'file': 'Tokenizer.test.cpp',
    },
    'Parser': {
        'libs': [ parser_lib, term_lib ],
        'file': 'Parser.test.cpp',
    },
    'AstGen': {
        'libs': [ astgen_lib ],
        'file': 'AstGen.test.cpp',
    },
    'Pool': {
        'libs': [ air_lib ],
        'file': 'Pool.test.cpp',
    },
}

foreach test_name, test_info : test_sources
    test_exec = executable('test-@0@'.format(test_name), test_info.get('file'),
        include_directories : inc,
        link_with : test_info.get('libs'),
        dependencies : doctest_dep
    )
    test('Test @0@'.format(test_name), test_exec)
endforeach

